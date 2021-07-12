# The following g++ options are those that are not enabled by
#
# -Wall -Wextra -Werror -Weffc++
#
# The above options are recommended to be in SContruct/Makefile, and the options
# below are enabled as much as practical. If a particular option causes
# false-positives in one or a few files, then the issue can be resolved by
# including "gcc_util.h" and surrounding the offending code by
# DISABLE_WARNING(abc) and ENABLE_WARNING(abc) where 'abc' is the name of the
# compiler option without leading '-W'.
#
# Feel free too add comments, per option, as 2nd, 3rd etc. item, or on a line
# above.
#
# NOTE 1: the set of options below is a complete set that complements the above
# options. Therefore, if a option is determined to be not useful, keep it in this
# file commented or add 'no-' after -W, but do not remove it. This way we
# maintain completeness and later might enable those that are commented. Removal
# of them would essentially hide them from future use.
#
# NOTE 2: Some very useful options are available for g++ ver 10 but not for g++
# ver 7; these are commented out and marked with 'N/A'. You can try using a
# newer compiler just for diagnostic purposes, and use the old one for
# building.
#
# Enjoy your clean code!
#

compiler_warnings = [

"-Wall",
"-Wextra",
"-Werror",
#"-Waggregate-return",         # not very useful
"-Waligned-new=all",
"-Walloc-zero",
"-Walloca",
#"-Wanalyzer-too-complex",     # N/A
#"-Warith-conversion",         # N/A
"-Warray-bounds=2",
#"-Wattribute-alias=2",        # N/A
"-Wcast-align",                # using =strict generates too many warnings
"-Wcast-qual",
#"-Wcatch-value=3",            # N/A
#"-Wclass-conversion",         # N/A
#"-Wclass-memaccess",          # N/A
#"-Wcomma-subscript",          # N/A
"-Wcomment",
"-Wconditionally-supported",
#"-Wconversion",               # too many cases: {,u}int{32,64}_t are commingled
"-Wconversion-null",
"-Wctor-dtor-privacy",
"-Wno-dangling-else",          # too many cases, therefore disabled
"-Wdate-time",
"-Wdelete-incomplete",
"-Wdelete-non-virtual-dtor",
#"-Wdeprecated-copy",          # N/A
#"-Wdeprecated-copy-dtor",     # N/A
"-Wdisabled-optimization",
#"-Wdouble-promotion",         # FOR NOW
#"-Wduplicated-branches",      # Useful, but g++ v 7 can give obscure messages
"-Wduplicated-cond",
"-Weffc++",
#"-Wextra-semi",               # N/A
#"-Wfloat-conversion",         # too many, disable FOR NOW
#"-Wfloat-equal",              # there are 3 cases, marked with 'FIXME'.
                               # Need 'fabs(x-y)<eps' instead
"-Wformat-nonliteral",
"-Wformat-overflow=2",
"-Wformat-security",
"-Wformat-signedness",
"-Wformat-truncation=2",
"-Wformat=2",                  # if too noisy, use '=1'
"-Wimplicit-fallthrough=5",    # use during dev to find issues.
                               # Otherwise fix some auto-genrated code
#"-Winaccessible-base",        # N/A
"-Winherited-variadic-ctor",
#"-Winit-list-lifetime",       # N/A
"-Winit-self",
#"-Winline",                   # g++ v 7: many false-positives, g++ v 10: OK
"-Winvalid-offsetof",
"-Winvalid-pch",
#"-Wlogical-op",               # Useful, but buggy in g++ v 7.
#"-Wmismatched-tags",          # N/A
"-Wmissing-declarations",
"-Wmissing-format-attribute",
"-Wmissing-include-dirs",
"-Wmultichar",
#"-Wmultiple-inheritance",     # useful to search, not needed in this lib
#"-Wmultiple-inheritance",     # useful to search, not needed in this lib
#"-Wnamespaces",               # useful to search, not needed in this lib
#"-Wnoexcept",                 # too many, FOR NOW
"-Wnoexcept-type",
"-Wnon-template-friend",
"-Wnon-virtual-dtor",
"-Wnormalized",
"-Wnormalized=nfkc",
#"-Wnull-dereference",         # g++ v 10: OK; g++ v 7: false-positives
"-Wold-style-cast",            # too many, need a separe PR to do static_cast<>
"-Woverlength-strings",
"-Woverloaded-virtual",
"-Wpacked",
#"-Wpadded",                   # useful info when designing packed structs.
                               # Turn off during dev

#"-Wpedantic                   # of academic interest
#"-Wpessimizing-move           # N/A
"-Wplacement-new=2",
"-Wpmf-conversions",
"-Wpointer-arith",
"-Wredundant-decls",           # there a few in generated code,
                               # so can't edit easily

#"-Wredundant-move",           # N/A
#"-Wredundant-tags",           # too many of these,
                               # especially 'struct tm tm;'
"-Wregister",
"-Wreorder",
"-Wscalar-storage-order",
"-Wshadow",
"-Wshift-overflow=2",
#"-Wsign-conversion",          # too many of these
#"-Wsign-promo",               # too many FOR NOW
"-Wsized-deallocation",
#"-Wstack-protector",          # useful for dev, but can gen fales-pos
"-Wstrict-aliasing=3",
"-Wstrict-null-sentinel",
"-Wno-strict-overflow",        # g++ v 10 : the =5 gives false-positives,
                               # using =4; g++ v 7: using disabling
"-Wstringop-overflow=4",
"-Wsubobject-linkage",
#"-Wsuggest-attribute=cold",   # N/A
#"-Wsuggest-attribute=const",  # g++ v 7: incorrect diag; g++ v 10 OK
"-Wsuggest-attribute=format",
#"-Wsuggest-attribute=malloc", # N/A

# This following option suggested to mark as '[[ noreturn ]]'
# a template member function", that always throws.
# Did this, and found the folowing: when a template",
# specialization for the above template function does
# not throw unconditionally,", it seems the specialization did not return.
# I.e. the compiler did not provide", for the return.
# So be careful when using this with templates.
# A non-template", functions marked '[[ noreturn ]]' work as expectd.

#"-Wsuggest-attribute=noreturn", # useful. Turn on to search for no-return functions.
#"-Wmissing-noreturn",           # useful. Turn on to search for no-return functions.
#"-Wsuggest-attribute=pure",   # too many (about 100) suggestions
#"-Wsuggest-final-methods",    # useful in g++ v 10;
                               # some false-positives in g++ v 7
#"-Wsuggest-final-types",      # some false-positives; useful to enable
                               #   temporarily
"-Wsuggest-override",
#"-Wswitch-default",           # useful to for checking, but there is
                               # generated code that is hard to fix
"-Wswitch-enum",
"-Wsynth",
#"-Wsystem-headers",           # of academic interest
#"-Wtemplates",                # useful to find template decl in user code
"-Wterminate",
"-Wtrampolines",
"-Wundef",
"-Wunreachable-code",
#"-Wunsafe-loop-optimizations", # g++ v 7: false-positive, g++ v 10: OK.
"-Wunused",
"-Wunused-const-variable=1",   # choice =2 complains about unused in the
                               # same translation unit
"-Wunused-macros",
"-Wuseless-cast",
"-Wvariadic-macros",
"-Wvector-operation-performance",
"-Wvirtual-inheritance",
"-Wvirtual-move-assign",
# "-Wvolatile",                # N/A
"-Wwrite-strings",
#"-Wzero-as-null-pointer-constant", # g++ ver 7 treats NULL as zero int,
                                    # it seems, usefule in g++ ver 10
]
