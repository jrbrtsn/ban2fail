/*****************************************************************************
 * In an effort to make C programming both easier and more secure, I present
 * the EZ method.
 *
 * Reasoning for this is simple; Many function calls can fail for a variety of
 * reasons, some or even most of which are obscure. However, it is both
 * insecure and counterproductive not to check for said failures. Unfortunately
 * coding proper checks and/or recovery from failures will involve (often
 * complex) logic constructs, and detracts from understanding the primary logic
 * of your source code.
 *
 * Sometimes your app needs to recover, and so you must supply this code, but
 * more often than not a "boilplate" approach is sufficient. The EZ method is a
 * consistent convention to supply centralized and reusable "boilerplate" error
 * handling on an _optional_ basis, both for existing library functions and
 * your own.
 *
 * Modern programming languages address this problem by throwing "exceptions",
 * which by default result in your program getting terminated. To avoid
 * termination you must write code to "catch" such exceptions, but this code
 * exists separated in source code from the function call, and tends to be both
 * confusing to read and hideous to look at. If you thought "goto" statements
 * were in poor taste, then this will make you vomit.
 *
 * Enter the EZ method; from a practical coding standpoint you merely prepend
 * "ez_" to any function call for which you desire default error handling. When
 * an error occurs a very helpful error message is printed (in DEBUG mode this
 * includes the source code location from which the function was called), and
 * your program is terminated. If your app needs to recover from such an error,
 * simply erase the "ez_" prefix from the function call and supply the recovery
 * code, where God intended it to be - right where you made the function call!
 *
 * In effect the EZ method provides the utility of throwing exceptions, but
 * without the catch - Woohoo!
 *****************************************************************************/

#ifndef EZ_H
#define EZ_H


/*****************************************************************************
 * These macros ease the drudgery of writing prototypes for the _ez_XXX()
 * wrappers of functions, and reduce the liklihood of bugs due to typing
 * errors.
 *
 * The elipses is for your function's argument list.
 *****************************************************************************/

#ifdef DEBUG

#define ez_proto(rtn, funcname, ...) \
   rtn _ez_##funcname( \
      const char *fileName, \
      int lineNo, \
      const char *funcName, \
      ##__VA_ARGS__ \
      )

#define ez_hdr_proto(rtn, funcname, ...) \
      ez_proto(rtn, funcname, ##__VA_ARGS__); \
      rtn funcname(__VA_ARGS__)

#else

#define ez_proto(rtn, funcname, ...) \
   rtn _ez_##funcname(__VA_ARGS__)

#define ez_hdr_proto(rtn, funcname, ...) \
      ez_proto(rtn, funcname, ##__VA_ARGS__); \
      rtn funcname(__VA_ARGS__)

#endif

/*****************************************************************************
 * For example, if you have the following function prototype:
 *
 *      int myfunc (struct foo *f, double bar)
 *
 * In the header file where the function prototype goes,
 * you will need this:
 * 
 *      ez_hdr_proto (int, myfunc, struct foo *f, double bar);
 *
 *      #ifdef DEBUG
 *      #       define ez_myfunc(...) \
 *           _ez_myfunc(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
 *      #else
 *      #       define ez_myfunc(...) \
 *          _ez_myfunc(__VA_ARGS__)
 *      #endif
 *              
 * This will expand to the standard function prototype as listed above, as well
 * as the _ez_myfunc() prototype correct for either DEBUG version or production
 * version, depending on whether or not DEBUG is #define'd. Also, there is now
 * a macro ez_myfunc() which will expand to the correct _ez_myfunc()
 * function call.
 *
 * In the implmentation (*.c) file, you will need to supply code for
 * _ez_myfunc(), which looks like so:
 *
 *      ez_proto(int, myfunc, struct foo *f, double bar)
 *      {
 *              // implementation goes here
 *      }
 *
 * Notice that the prototype section is is the same as what you placed in the
 * corresponding header file, minus "_hdr" in the ez_hdr_proto() macro.
 *
 * For the case where you are supplying only the ez_XXX() version of an
 * existing function found in some library 'liba2z', you can place the
 * following in your own header file, "ez_liba2z.h":
 *
 *      ez_proto(struct rtnType*, liba2z_funcname, struct foo *f, double bar);
 *
 * Everything else works similar to the previous examples.
 * Now you can use the EZ error handling like so:
 *
 * #include "ez_liba2z.h"
 * int main(int argc, char **argv)
 * {
 *    struct foo Foo;
 *    double bar;
 *    struct rtnType *rtn= ez_liba2z_funcname(&Foo, bar);
 * }
 *
 *****************************************************************************/

#endif
