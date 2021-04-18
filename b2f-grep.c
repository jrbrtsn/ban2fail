#include <assert.h>
#include <arpa/inet.h>
#include <byteswap.h>
#include <getopt.h>
#include <gmp.h>

#include "ez_libc.h"
#include "ptrvec.h"
#include "util.h"

/*==================================================================*/
/*=================== Support structs ==============================*/
/*==================================================================*/

/****************************************************************
 * signed 129 bit integer IP address or IP address difference. 
 * This covers IPv4 and IPv6 addresses.
 */
typedef struct _MpAddr {

   enum {
      MPADDR_INIT_FLG= 1<<0
   } flags;

   // AF_INET or AF_INET6
   int domain;

   // GNU multi-precision integer
   mpz_t addr;

} MpAddr;

/****************************************************************
 * Entry class, one for each address passed on command line.
 */
/* Useful class for this app */
typedef struct _Entry {

   // 129 bit integer address.
   MpAddr mp_addr;

   // Line from stdin which has was closest to ours.
   unsigned lineno;

    // Invalid if 0 == lineno
   MpAddr mp_closest_addr, // Address which was closest to ours.
          mp_closest_prox; // Proximity to ours.

} Entry;

/*==================================================================*/
/*================= Forward declarations ===========================*/
/*==================================================================*/

static void MpAddr_sinit(MpAddr *self);
static MpAddr* MpAddr_constructor(MpAddr *self);
static void* MpAddr_destructor(MpAddr *self);
static int MpAddr_inetAssign(MpAddr *self, const char *addrStr);
static void MpAddr_assign(MpAddr *self, const MpAddr *src);
static void MpAdd_absDiff(MpAddr *self, const MpAddr *opl, const MpAddr *opr);
static int MpAdd_cmp(const MpAddr *self, const MpAddr *other);
static int MpAddr_isSameFamily(const MpAddr *self, const MpAddr *other);
static int MpAddr_lt(const MpAddr *self, const MpAddr *other);


#define Entry_create(p, addr) \
  ((p)=(Entry_constructor((p)=malloc(sizeof(Entry)), addr) ? (p) : ( p ? realloc(Entry_destructor(p),0) : 0 )))
static Entry* Entry_constructor(Entry *self, const char *addr);
static void* Entry_destructor(Entry *self);
//static int Entry_isSameFamily(const Entry *self, const MpAddr *mp_addr);
static int Entry_is_closer(const Entry *self, const MpAddr *mp_addr);
static int Entry_lineno_cmp(const void *const*pp1, const void *const* pp2);
static void Entry_print(const Entry *self);
static void Entry_register(Entry *self, unsigned lineno, const MpAddr *mp_addr);
//static MpAddr* Entry_get_prox(const Entry *self, MpAddr *rtn, const MpAddr *other);

/*==================================================================*/
/*========================= static data ============================*/
/*==================================================================*/

static struct {

   PTRVEC entry_vec;

   unsigned max_off;

   struct {
      int major,
          minor,
          patch;
   } version;

} S= {
   .max_off= 50,

   .version= {
      .major= 0,
      .minor= 0,
      .patch= 0
   }
};

/*==================================================================*/
/*======================== main() stuff ============================*/
/*==================================================================*/
/* Enums for long options */
enum {
   VERSION_OPT_ENUM=128, /* Larger than any printable character */
   HELP_OPT_ENUM
};

/*==================================================================*/
/*======================== main() ==================================*/
/*==================================================================*/
int
main(int argc, char **argv)
/***************************************************
 * Program execution begins here ...
 */
{
   int rtn= EXIT_FAILURE;

   extern char *optarg;
   extern int optind, optopt;

   PTRVEC_constructor(&S.entry_vec, 10);


   /* Parse command line arguments */
   { /*==================================================================*/
      int c, errflg= 0;

      for(;;) {

         static const struct option long_options[]= {
            {"help", no_argument, 0, HELP_OPT_ENUM},
            {"version", no_argument, 0, VERSION_OPT_ENUM},
            {}
         };

         int c, option_ndx= 0;

         c= getopt_long(argc, argv, ":m:", long_options, &option_ndx);

         if(-1 == c) break;

        switch(c) {

            /* print usage help */
            case HELP_OPT_ENUM:
               ++errflg;
               break;

            case VERSION_OPT_ENUM:
               ez_fprintf(stdout, "b2f-grep v%d.%d.%d\n", S.version.major, S.version.minor, S.version.patch);
               return 0;

            case 'm':
               {
                  int rc= sscanf(optarg, "%u", &S.max_off);
                  if(1 != rc)
                     ++errflg;
               } break;

            case '?':
               ez_fprintf(stderr, "Unrecognized option: -%c\n", optopt);
               ++errflg;
               break;
         }
      }

      if(errflg) {
         ez_fprintf(stderr, 
"b2f-grep v%d.%d.%d Usage:\n"
"%s [options] addr1 [...] <ban2fail.cfg_file\n"
"  -m max\tlargest value to consider in MAX_OFFENSES block\n"
"  --help\tprint this usage message.\n"
"  --version\tprint the version number and exit.\n"
         , S.version.major, S.version.minor, S.version.patch
         , argv[0]
         );

         goto abort;
      }

      /* Make sure we have at least one address to process */
      if(optind == argc) {
         eprintf("ERROR: no addresses supplied on command line.");
         goto abort;
      }

      /* Pick up addresses on command line */
      for(; optind < argc; ++optind) {

         const char *addr= argv[optind];

         /* Create an entry, and place it in our vector */
         Entry *e;
         if(!Entry_create(e, addr))
            continue;
         PTRVEC_addTail(&S.entry_vec, e);
      }

   } // command options

   { /*====== Line by line processing ============*/
      static char lbuf[1024],
                  ipbuf[40];
      static MpAddr mp_addr,
                    mp_prox;
      MpAddr_sinit(&mp_addr);
      MpAddr_sinit(&mp_prox);

      char *str;
      unsigned lineno, max_off= 0;
      for(lineno= 1, str= ez_fgets(lbuf, sizeof(lbuf)-1, stdin);
          str;
          str= ez_fgets(lbuf, sizeof(lbuf)-1, stdin), ++lineno)
      {
         str= trim(str);

         /* See if this is a MAX_OFFENSES block declaration */
         int rc= sscanf(str, "MAX_OFFENSES %u", &max_off);
         if(1 == rc || max_off > S.max_off) 
            continue; // We have what we need for this line

         /* Nope, maybe an IP address ... */
         rc= sscanf(str, "IP = %39[^ \t]", ipbuf);
         if(1 != rc)
            continue; // Ignore line

         if(MpAddr_inetAssign(&mp_addr, ipbuf)) {
            eprintf("ERROR: on line %u", lineno);
            continue; // Don't recognize address string
         }

         { /* Loop through all entries, assign line numbers if necessary */
            unsigned i;
            Entry *e;
            PTRVEC_loopFwd(&S.entry_vec, i, e) {
               Entry_register(e, lineno, &mp_addr);
            }
         }
      }
   } // reading stdin

   { /*========= Print results ============*/
      unsigned i;
      Entry *e;
      PTRVEC_sort(&S.entry_vec, Entry_lineno_cmp);
      PTRVEC_loopFwd(&S.entry_vec, i, e) {
         if(i)
            ez_fputc('\t', stdout);
         Entry_print(e);
      }
      ez_fputc('\n', stdout);
   }

   rtn= EXIT_SUCCESS;
abort:
   return rtn;
}

/*==================================================================*/
/*============== Supporting functions ==============================*/
/*==================================================================*/

static void
MpAddr_sinit(MpAddr *self)
/**************************************************************
 * Construct if necessary.
 */
{
   if(!(self->flags & MPADDR_INIT_FLG))
      MpAddr_constructor(self);
}

static MpAddr*
MpAddr_constructor(MpAddr *self)
/**************************************************************
 * Initialize for use.
 */
{

   mpz_init2(self->addr, 129);

   self->flags= MPADDR_INIT_FLG;
   return self;
}

static void*
MpAddr_destructor(MpAddr *self)
/**************************************************************
 * Free resources.
 */
{
   if(self->flags & MPADDR_INIT_FLG)
      mpz_clear(self->addr);

   return self;
}

static int
MpAddr_inetAssign(MpAddr *self, const char *addrStr)
/**************************************************************
 * Assign address from string representation of numeric address
 */
{
   int rtn= -1;

   char hex_buf[33];
   unsigned nBytes;
   unsigned char buf[sizeof(struct in6_addr)];

   /* Determine if this is ipv6 or ipv6 */
   self->domain= strchr(addrStr, ':') ? AF_INET6 : AF_INET;

   /* Convert to a big-endian integer of 4 or 16 bytes */
   int rc= inet_pton(self->domain, addrStr, buf);
   switch(rc) {
      case -1:
         sys_eprintf("ERROR: inet_pton(\"%s\")", addrStr);
         goto abort;

      case 0:
         eprintf("WARNING: \"%s\" not recognized as an address", addrStr);
         goto abort;
   }


   /* At this point we have the address as an integer in big-endian order in buf */
   switch(self->domain) {

      case AF_INET:
         nBytes= 4;
         break;

      case AF_INET6:
         nBytes= 16;
         break;

      default:
         assert(0);
   }

   /* Convert big-endian integer we have in buf[] to hexidecimal string */
   for(unsigned i= 0; i < nBytes; ++i)
      snprintf(hex_buf+2*i, sizeof(hex_buf) - 2*i, "%02hhu", buf[i]);

   /* Set multi-precision integer to hex string value */
   rc= mpz_set_str(self->addr, hex_buf, 16);
   if(-1 == rc) {
      eprintf("ERROR: \"%s\" not recognized as hexidecimal integer.", hex_buf);
      goto abort;
   }

   rtn= 0;
abort:
   return rtn;
}

static void
MpAdd_absDiff(MpAddr *self, const MpAddr *opl, const MpAddr *opr)
/***************************************************************
 * Compute the absolute different between opl and opr
 */
{
   MpAddr diff;
   MpAddr_sinit(&diff);

   mpz_sub(diff.addr, opl->addr, opr->addr);
   mpz_abs(self->addr, diff.addr);

}

static int
MpAdd_cmp(const MpAddr *self, const MpAddr *other)
/***************************************************************
 * strcmp() style comparision return.
 */
{
   return mpz_cmp(self->addr, other->addr);

}

static int
MpAddr_isSameFamily(const MpAddr *self, const MpAddr *other)
/***************************************************************
 * Returns non-zero if self and other are in the same address
 * family.
 */
{
   return self->domain == other->domain;
}

static int
MpAddr_cmp(const MpAddr *self, const MpAddr *other)
/***************************************************************
 * strcmp() style return value for comparison of self & other.
 */
{
   return mpz_cmp(self->addr, other->addr);
}

static void
MpAddr_assign(MpAddr *self, const MpAddr *src)
/***************************************************************
 * Assign the value of src to ourself.
 */
{
   mpz_set(self->addr, src->addr);
}

static Entry*
Entry_constructor(Entry *self, const char *addr)
/***************************************************************
 * Create an entry object for the supplied IP address string.
 */
{
   memset(self, 0, sizeof(*self));

   Entry *rtn= NULL;

   MpAddr_constructor(&self->mp_addr);
   MpAddr_constructor(&self->mp_closest_addr);
   MpAddr_constructor(&self->mp_closest_prox);

   if(MpAddr_inetAssign(&self->mp_addr, addr))
      goto abort;

   rtn= self;

abort:
   return rtn;
}

static void*
Entry_destructor(Entry *self)
/***************************************************************
 * Destructor.
 */
{
   MpAddr_destructor(&self->mp_addr);
   MpAddr_destructor(&self->mp_closest_addr);
   MpAddr_destructor(&self->mp_closest_prox);
   return self;
}

static void
Entry_print(const Entry *self)
/***************************************************************
 * Print result to stdout.
 */
{
   unsigned lineno= MAX(self->lineno, 1);
   if(MpAddr_cmp(&self->mp_addr, &self->mp_closest_addr))
      --lineno;
   ez_fprintf(stdout, "%u", lineno);
}

static int
Entry_lineno_cmp(const void *const*pp1, const void *const* pp2)
/***************************************************************
 * Comparison function we can use with PTRVEC_sort().
 */
{
   const Entry *e1= *(const Entry *const*)pp1,
               *e2= *(const Entry *const*)pp2;

   if(e1->lineno < e2->lineno)
      return -1;
   if(e1->lineno > e2->lineno)
      return 1;
   return 0;
      return -1;
}

static void
Entry_register(Entry *self, unsigned lineno, const MpAddr *mp_addr)
/***************************************************************
 * Registers the address found on lineno.
 */
{
   /* Ignore address of different family */
   if(!MpAddr_isSameFamily(&self->mp_addr, mp_addr))
      return;

   static MpAddr prox;
   MpAddr_sinit(&prox);

   MpAdd_absDiff(&prox, &self->mp_addr, mp_addr);

   if(!self->lineno || 0 < MpAddr_cmp(&self->mp_closest_prox, &prox)) {
      self->lineno= lineno;
      MpAddr_assign(&self->mp_closest_addr, mp_addr);
      MpAddr_assign(&self->mp_closest_prox, &prox);
   }
}

static int
Entry_isSameFamily(const Entry *self, const MpAddr *mp_addr)
/***************************************************************
 * Frontend for MpAddr_isSameFamily()
 */
{
   return MpAddr_isSameFamily(&self->mp_addr, mp_addr);
}

