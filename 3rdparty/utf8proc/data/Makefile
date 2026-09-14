# Unicode data generation rules.  Except for the test data files, most
# users will not use these Makefile rules, which are primarily to re-generate
# unicode_data.c when we get a new Unicode version or charwidth data; they
# require julia to be installed.

# programs
CURL=curl
PERL=perl
MAKE=make
JULIA=julia
CURLFLAGS = --retry 5 --location

.PHONY: clean

.DELETE_ON_ERROR:

RAWDATA = UnicodeData.txt GraphemeBreakProperty.txt DerivedCoreProperties.txt CompositionExclusions.txt CaseFolding.txt EastAsianWidth.txt emoji-data.txt

utf8proc_data.c.new: data_generator.jl $(RAWDATA)
	$(JULIA) --project=. -e 'using Pkg; Pkg.instantiate()'
	$(JULIA) --project=. data_generator.jl > $@

# Unicode data version (must also update utf8proc_unicode_version function)
UNICODE_VERSION=17.0.0

UnicodeData.txt:
	$(CURL) $(CURLFLAGS) -o $@ https://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/UnicodeData.txt

EastAsianWidth.txt:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/EastAsianWidth.txt

GraphemeBreakProperty.txt:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/auxiliary/GraphemeBreakProperty.txt

DerivedCoreProperties.txt:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/DerivedCoreProperties.txt

CompositionExclusions.txt:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/CompositionExclusions.txt

CaseFolding.txt:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/CaseFolding.txt

NormalizationTest.txt:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/NormalizationTest.txt

GraphemeBreakTest.txt:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/auxiliary/GraphemeBreakTest.txt

emoji-data.txt:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://unicode.org/Public/$(UNICODE_VERSION)/ucd/emoji/emoji-data.txt

Uppercase.txt: DerivedCoreProperties.txt
	$(JULIA) -e 'print(match(r"# Derived Property: Uppercase.*?# Total code points:"s, read("DerivedCoreProperties.txt", String)).match)' > $@

Lowercase.txt: DerivedCoreProperties.txt
	$(JULIA) -e 'print(match(r"# Derived Property: Lowercase.*?# Total code points:"s, read("DerivedCoreProperties.txt", String)).match)' > $@

clean:
	rm -f $(RAWDATA) NormalizationTest.txt GraphemeBreakTest.txt
	rm -f Uppercase.txt Lowercase.txt
	rm -f utf8proc_data.c.new
