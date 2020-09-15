baseDir := ~
libsDir := $(baseDir)/libs
projectName := ban2fail
versions := debug release
cc_exe := ban2fail fsckdns
#install_dir := /usr/local/bin

########################################
# Set up sources & libraries here.     #
########################################

ifeq ($(exe), ban2fail)
src := \
       addrRpt.c \
       ban2fail.c \
       cfgmap.c \
       cntry.c \
       dynstack.c \
       es.c \
       ez_libc.c \
       ez_libdb.c \
       ez_libpthread.c \
       ez_libz.c \
       iptables.c \
       logType.c \
       logFile.c \
       map.c \
       maxoff.c \
       msgqueue.c \
       obsvTpl.c \
       offEntry.c \
       pdns.c \
       ptrvec.c \
       str.c \
       target.c \
       timestamp.c \
       util.c \

   libs := z crypto GeoIP pthread db
endif

ifeq ($(exe), fsckdns)
src := \
       ez_libc.c \
       fsckdns.c \
       str.c \
       util.c \

#   libs := z crypto GeoIP pthread
endif

########################################
# Set up custom compile flags here.    #
########################################
ifeq ($(version),debug)
local_cppflags := $(local_cppflags) -D_DEBUG -DDEBUG -std=gnu99
local_codeflags := -g2 -O0 -Wreturn-type -Wformat -Wchar-subscripts -Wparentheses -Wcast-qual -Wmissing-declarations
local_ldflags := $(local_ldflags) -L$(libsDir)/$(version)
endif

ifeq ($(version),release)
local_cppflags := $(local_cppflags) -DNDEBUG -std=gnu99
local_codeflags :=  -g0 -O3 -Wreturn-type -Wformat -Wchar-subscripts -Wparentheses -Wcast-qual -Wmissing-declarations
local_ldflags := $(local_ldflags) -L$(libsDir)/$(version)
endif

makefile := Makefile
ifndef version
.PHONY : all clean tidy install uninstall debug release
all :  debug release
debug  :
	@$(MAKE) version=debug exe=ban2fail mainType=CC --no-builtin-rules -f $(makefile) --no-print-directory
	@$(MAKE) version=debug exe=fsckdns mainType=CC --no-builtin-rules -f $(makefile) --no-print-directory
release  :
	@$(MAKE) version=release exe=ban2fail mainType=CC --no-builtin-rules -f $(makefile) --no-print-directory
	@$(MAKE) version=release exe=fsckdns mainType=CC --no-builtin-rules -f $(makefile) --no-print-directory
install : release
	@strip release/ban2fail
	@[ $(install_dir)_foo = _foo ] || cp release/ban2fail $(install_dir)/
	@strip release/fsckdns
	@[ $(install_dir)_foo = _foo ] || cp release/fsckdns $(install_dir)/
	@[ -e install.sh ] && INSTALLDIR=$(install_dir) INSTALLTYPE=$(install_type) sudo ./install.sh
uninstall :
clean :
	$(RM) -r $(versions) core *.bak *.tab.h *.tab.c *.yy.c *.yy.h
tidy :
	$(RM) $(foreach vs, $(versions), $(vs)/*.o $(vs)/*.d) core *.bak
endif
.DELETE_ON_ERROR :

ifdef version
roots := \
 $(patsubst %.cc, %, $(filter %.cc, $(src)))\
 $(patsubst %.cxx, %, $(filter %.cxx, $(src)))\
 $(patsubst %.cpp, %, $(filter %.cpp, $(src)))\
 $(patsubst %.C, %, $(filter %.C, $(src)))\
 $(patsubst %.c, %, $(filter %.c, $(src)))\
 $(patsubst %.f, %, $(filter %.f, $(src)))\
 $(patsubst %.for, %, $(filter %.for, $(src)))\
 $(patsubst %.sal, %, $(filter %.sal, $(src)))\
 $(patsubst %.asm, %, $(filter %.asm, $(src)))\
 $(patsubst %.h, qt_%, $(filter %.h, $(src)))

yacc_roots := $(patsubst %.y, %.tab, $(filter %.y, $(src)))
lex_roots := $(patsubst %.l, %.yy, $(filter %.l, $(src)))
obj := $(patsubst %, $(version)/%.o, $(roots) $(yacc_roots) $(lex_roots))
dep := $(patsubst %, $(version)/%.d, $(roots) $(yacc_roots) $(lex_roots))


ifdef exe #>>>>>>>>>>>> We are building an executable <<<<<<<<<<<<<<<<

ifndef mainType
$(version)/$(exe) : $(obj)
	@echo 'THE VARIABLE "mainType" MUST BE DEFINED TO: CXX or CC or FC'
endif

ifeq ($(mainType),CXX)
$(version)/$(exe) : $(obj)
	$(CXX) $(LDFLAGS) $(local_ldflags) $(obj) $(patsubst %, -l%, $(libs)) -o $@
endif # ifeq CXX

ifeq ($(mainType),CC)
$(version)/$(exe) : $(obj)
	$(CC) $(LDFLAGS) $(local_ldflags) $(obj) $(patsubst %, -l%, $(libs)) -o $@
endif # ifeq CC

ifeq ($(mainType),FC)
$(version)/$(exe) : $(obj)
	$(FC) $(LDFLAGS) $(local_ldflags) $(obj) $(patsubst %, -l%, $(libs)) -o $@
endif # ifeq FC
endif # ifdef exe


ifdef library #>>>>>>>>>>>> We are building a library <<<<<<<<<<<<<<<<
ifeq ($(libType),STATIC)
ifdef libsDir
$(libsDir)/$(version)/lib$(library).a : $(version)/lib$(library).a
	@[ -d $(libsDir)/$(version) ] || mkdir -p $(libsDir)/$(version)
	@ln -f -s `pwd`/$(version)/lib$(library).a $(libsDir)/$(version)/lib$(library).a

endif # ifdef libsDir

$(version)/lib$(library).a : $(obj)
	$(AR) $(ARFLAGS) $@ $(obj)
endif # ifeq STATIC

ifeq ($(libType),SHARED)
ifdef libsDir
$(libsDir)/$(version)/lib$(library) : $(version)/lib$(library)
	@[ -d $(libsDir)/$(version) ] || mkdir -p $(libsDir)/$(version)
	@ln -f -s `pwd`/$(version)/lib$(library) $(libsDir)/$(version)/lib$(library)

endif # ifdef libsDir
$(version)/lib$(library) : $(obj)
	g++ -shared -Wl,-soname,lib$(library) -o $@ $(obj)

local_codeflags += -fno-strength-reduce -fPIC
endif # ifeq SHARED

endif # ifdef library
#>>>>>>>>>>>>>>>>>>>> Finished library specific stuff <<<<<<<<<<<<<<<<<

# yacc stuff
yacc_h_output := $(patsubst %, %.h, $(yacc_roots))
yacc_c_output := $(patsubst %, %.c, $(yacc_roots))
yacc_output := $(yacc_h_output) $(yacc_c_output)

%.tab.c : %.y
	bison -d $< 
%.tab.h : %.y
	bison -d $< 

# lex stuff
lex_h_output := $(patsubst %, %.h, $(lex_roots))
lex_c_output := $(patsubst %, %.c, $(lex_roots))
lex_output := $(lex_h_output) $(lex_c_output)

%.yy.c: %.l
	flex -o $*.yy.c --header-file=$*.yy.h $< 
%.yy.h: %.l
	flex -o $*.yy.c --header-file=$*.yy.h $< 

# Make sure the build directory exists
$(dep) : | $(version)

$(version) :
	@mkdir $(version)

# Dependency files rule
$(dep) : $(yacc_output) $(lex_output)

# Recipes to build .d files
$(version)/%.d: %.cc
	@set -e; $(CXX) -M $(CPPFLAGS) $(local_cppflags) $< \
         | sed 's/\($*\)\.o[ :]*/$(version)\/\1.o $(version)\/\1.d : /' > $@
$(version)/%.d: %.cxx
	@set -e; $(CXX) -M $(CPPFLAGS) $(local_cppflags) $< \
         | sed 's/\($*\)\.o[ :]*/$(version)\/\1.o $(version)\/\1.d : /' > $@
$(version)/%.d: %.cpp
	@set -e; $(CXX) -M $(CPPFLAGS) $(local_cppflags) $< \
         | sed 's/\($*\)\.o[ :]*/$(version)\/\1.o $(version)\/\1.d : /' > $@
$(version)/%.d: %.C
	@set -e; $(CXX) -M $(CPPFLAGS) $(local_cppflags) $< \
         | sed 's/\($*\)\.o[ :]*/$(version)\/\1.o $(version)\/\1.d : /' > $@
$(version)/%.d: %.c
	@set -e; $(CC) -M $(CPPFLAGS) $(local_cppflags) $< \
         | sed 's/\($*\)\.o[ :]*/$(version)\/\1.o $(version)\/\1.d : /' > $@

$(version)/%.d: %.f
	@echo $(patsubst %.f, $(version)/%.o, $<) : $< > $@

$(version)/%.d: %.for
	@echo $(patsubst %.for, $(version)/%.o, $<) : $< > $@

$(version)/qt_%.d: %.h
	@echo $(patsubst %.h, $(version)/qt_%.cxx, $<) : $< > $@

$(version)/%.d: %.sal
	@echo $(patsubst %.sal, $(version)/%.s, $<) : $< > $@

$(version)/%.d: %.asm
	@echo $(patsubst %.asm, $(version)/%.s, $<) : $< > $@

# The .d files contain specific prerequisite dependencies
-include $(patsubst %, $(version)/%.d, $(roots) $(yacc_roots) $(lex_roots))

# Recipes to build object files
$(version)/%.o: %.cc
	$(CXX) -c $(CXXFLAGS) $(local_codeflags) $(CPPFLAGS) $(local_cppflags) $< -o $@
$(version)/%.o: %.cxx
	$(CXX) -c $(CXXFLAGS) $(local_codeflags) $(CPPFLAGS) $(local_cppflags) $< -o $@
$(version)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(local_codeflags) $(CPPFLAGS) $(local_cppflags) $< -o $@
$(version)/%.o: %.C
	$(CXX) -c $(CXXFLAGS) $(local_codeflags) $(CPPFLAGS) $(local_cppflags) $< -o $@
$(version)/%.o: %.c
	$(CC) -c $(CCFLAGS) $(local_codeflags) $(CPPFLAGS) $(local_cppflags) $< -o $@
$(version)/%.o: %.f
	$(FC) -c $(FFLAGS) $(local_codeflags) $< -o $@
$(version)/%.o: %.for
	$(FC) -c $(FFLAGS) $(local_codeflags) $< -o $@
$(version)/qt_%.o: %.h
	$(QTDIR)/bin/moc $< -o $(version)/qt_$*.cxx
	$(CXX) -c $(CXXFLAGS) $(local_codeflags) $(CPPFLAGS) $(local_cppflags) $(version)/qt_$*.cxx -o $(version)/qt_$*.o

endif # version
