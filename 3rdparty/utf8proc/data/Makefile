# Unicode data generation rules.  Except for the test data files, most
# users will not use these Makefile rules, which are primarily to re-generate
# unicode_data.c when we get a new Unicode version or charwidth data; they
# require ruby, fontforge, and julia to be installed.

# programs
CURL=curl
RUBY=ruby
PERL=perl
MAKE=make
JULIA=julia
FONTFORGE=fontforge
CURLFLAGS = --retry 5 --location

.PHONY: clean

.DELETE_ON_ERROR:

utf8proc_data.c.new: data_generator.rb UnicodeData.txt GraphemeBreakProperty.txt DerivedCoreProperties.txt CompositionExclusions.txt CaseFolding.txt CharWidths.txt
	$(RUBY) data_generator.rb < UnicodeData.txt > $@

# GNU Unifont version for font metric calculations:
UNIFONT_VERSION=9.0.04

unifont.ttf:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://mirrors.kernel.org/gnu/unifont/unifont-$(UNIFONT_VERSION)/unifont-$(UNIFONT_VERSION).ttf

unifont_upper.ttf:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://mirrors.kernel.org/gnu/unifont/unifont-$(UNIFONT_VERSION)/unifont_upper-$(UNIFONT_VERSION).ttf

%.sfd: %.ttf
	$(FONTFORGE) -lang=ff -c "Open(\"$<\");Save(\"$@\");Quit(0);"

CharWidths.txt: charwidths.jl unifont.sfd unifont_upper.sfd EastAsianWidth.txt
	$(JULIA) charwidths.jl > $@

# Unicode data version
UNICODE_VERSION=9.0.0

UnicodeData.txt:
	$(CURL) $(CURLFLAGS) -o $@ -O $(URLCACHE)http://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/UnicodeData.txt

EastAsianWidth.txt:
	$(CURL) $(CURLFLAGS) -o $@ -O $(URLCACHE)http://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/EastAsianWidth.txt

GraphemeBreakProperty.txt:
	$(CURL) $(CURLFLAGS) -o $@ -O $(URLCACHE)http://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/auxiliary/GraphemeBreakProperty.txt

DerivedCoreProperties.txt:
	$(CURL) $(CURLFLAGS) -o $@ -O $(URLCACHE)http://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/DerivedCoreProperties.txt

CompositionExclusions.txt:
	$(CURL) $(CURLFLAGS) -o $@ -O $(URLCACHE)http://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/CompositionExclusions.txt

CaseFolding.txt:
	$(CURL) $(CURLFLAGS) -o $@ -O $(URLCACHE)http://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/CaseFolding.txt

NormalizationTest.txt:
	$(CURL) $(CURLFLAGS) -o $@ -O $(URLCACHE)http://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/NormalizationTest.txt

GraphemeBreakTest.txt:
	$(CURL) $(CURLFLAGS) $(URLCACHE)http://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/auxiliary/GraphemeBreakTest.txt | $(PERL) -pe 's,÷,/,g;s,×,+,g' > $@

clean:
	rm -f UnicodeData.txt EastAsianWidth.txt GraphemeBreakProperty.txt DerivedCoreProperties.txt CompositionExclusions.txt CaseFolding.txt NormalizationTest.txt GraphemeBreakTest.txt CharWidths.txt unifont*.ttf unifont*.sfd
	rm -f utf8proc_data.c.new
