# Makefile for Sphinx documentation
#

# You can set these variables from the command line.
SPHINXOPTS    =
SPHINXBUILD   = sphinx-build
PAPER         =
BUILDDIR      = build

# Internal variables.
PAPEROPT_a4     = -D latex_paper_size=a4
PAPEROPT_letter = -D latex_paper_size=letter
ALLSPHINXOPTS   = -d $(BUILDDIR)/doctrees $(PAPEROPT_$(PAPER)) $(SPHINXOPTS) source
# the i18n builder cannot share the environment and doctrees with the others
I18NSPHINXOPTS  = $(PAPEROPT_$(PAPER)) $(SPHINXOPTS) source

.PHONY: help clean html dirhtml singlehtml pickle json htmlhelp qthelp devhelp epub latex latexpdf text man changes linkcheck doctest gettext

ifeq ($(OS), Windows_NT)
define blankline =
@echo.
endef
else
define blankline =
@echo
endef
endif

help:
	$(blankline)
	@echo Please use make [target] where [target] is one of:
	$(blankline)
ifeq ($(OS), Windows_NT)
	@echo   html       to make standalone HTML files
	@echo   dirhtml    to make HTML files named index.html in directories
	@echo   singlehtml to make a single large HTML file
	@echo   pickle     to make pickle files
	@echo   json       to make JSON files
	@echo   htmlhelp   to make HTML files and a HTML help project
	@echo   qthelp     to make HTML files and a qthelp project
	@echo   devhelp    to make HTML files and a Devhelp project
	@echo   epub       to make an epub
	@echo   latex      to make LaTeX files, you can set PAPER=a4 or PAPER=letter
	@echo   latexpdf   to make LaTeX files and run them through pdflatex
	@echo   text       to make text files
	@echo   man        to make manual pages
	@echo   texinfo    to make Texinfo files
	@echo   info       to make Texinfo files and run them through makeinfo
	@echo   gettext    to make PO message catalogs
	@echo   changes    to make an overview of all changed/added/deprecated items
	@echo   linkcheck  to check all external links for integrity
	@echo   doctest    to run all doctests embedded in the documentation (if enabled)
else
	@echo "  html       to make standalone HTML files"
	@echo "  dirhtml    to make HTML files named index.html in directories"
	@echo "  singlehtml to make a single large HTML file"
	@echo "  pickle     to make pickle files"
	@echo "  json       to make JSON files"
	@echo "  htmlhelp   to make HTML files and a HTML help project"
	@echo "  qthelp     to make HTML files and a qthelp project"
	@echo "  devhelp    to make HTML files and a Devhelp project"
	@echo "  epub       to make an epub"
	@echo "  latex      to make LaTeX files, you can set PAPER=a4 or PAPER=letter"
	@echo "  latexpdf   to make LaTeX files and run them through pdflatex"
	@echo "  text       to make text files"
	@echo "  man        to make manual pages"
	@echo "  texinfo    to make Texinfo files"
	@echo "  info       to make Texinfo files and run them through makeinfo"
	@echo "  gettext    to make PO message catalogs"
	@echo "  changes    to make an overview of all changed/added/deprecated items"
	@echo "  linkcheck  to check all external links for integrity"
	@echo "  doctest    to run all doctests embedded in the documentation (if enabled)"
endif

clean:
	-rm -rf $(BUILDDIR)/*

site: html epub latexpdf
	-mkdir build/html/_files
	-cp build/epub/MAMEDocumentation.epub build/html/_files/MAME.epub
	-cp build/latex/MAME.pdf build/html/_files/MAME.pdf

html:
	$(SPHINXBUILD) -b html $(ALLSPHINXOPTS) $(BUILDDIR)/html
	$(blankline)
	@echo Build finished.

dirhtml:
	$(SPHINXBUILD) -b dirhtml $(ALLSPHINXOPTS) $(BUILDDIR)/dirhtml
	$(blankline)
	@echo Build finished.

singlehtml:
	$(SPHINXBUILD) -b singlehtml $(ALLSPHINXOPTS) $(BUILDDIR)/singlehtml
	$(blankline)
	@echo Build finished.

pickle:
	$(SPHINXBUILD) -b pickle $(ALLSPHINXOPTS) $(BUILDDIR)/pickle
	$(blankline)
	@echo Build finished.

json:
	$(SPHINXBUILD) -b json $(ALLSPHINXOPTS) $(BUILDDIR)/json
	$(blankline)
	@echo Build finished.

htmlhelp:
	$(SPHINXBUILD) -b htmlhelp $(ALLSPHINXOPTS) $(BUILDDIR)/htmlhelp
	$(blankline)
	@echo Build finished.

qthelp:
	$(SPHINXBUILD) -b qthelp $(ALLSPHINXOPTS) $(BUILDDIR)/qthelp
	$(blankline)
	@echo Build finished.

devhelp:
	$(SPHINXBUILD) -b devhelp $(ALLSPHINXOPTS) $(BUILDDIR)/devhelp
	$(blankline)
	@echo Build finished.

epub:
	$(SPHINXBUILD) -b epub $(ALLSPHINXOPTS) $(BUILDDIR)/epub
	$(blankline)
	@echo Build finished.

latex:
	$(SPHINXBUILD) -b latex $(ALLSPHINXOPTS) $(BUILDDIR)/latex
	$(blankline)
	@echo Build finished.

latexpdf:
	$(SPHINXBUILD) -b latex $(ALLSPHINXOPTS) $(BUILDDIR)/latex
	@echo Running LaTeX files through pdflatex...
	$(MAKE) -C $(BUILDDIR)/latex all-pdf
	$(blankline)
	@echo Build pdflatex finished; the PDF files are in $(BUILDDIR)/latex.

text:
	$(SPHINXBUILD) -b text $(ALLSPHINXOPTS) $(BUILDDIR)/text
	$(blankline)
	@echo Build finished.

man:
	$(SPHINXBUILD) -b man $(ALLSPHINXOPTS) $(BUILDDIR)/man
	$(blankline)
	@echo Build finished.

texinfo:
	$(SPHINXBUILD) -b texinfo $(ALLSPHINXOPTS) $(BUILDDIR)/texinfo
	$(blankline)
	@echo Build finished.

info:
	$(SPHINXBUILD) -b texinfo $(ALLSPHINXOPTS) $(BUILDDIR)/texinfo
	@echo Running Texinfo files through makeinfo...
	make -C $(BUILDDIR)/texinfo info
	$(blankline)
	@echo Build makeinfo finished; the Info files are in $(BUILDDIR)/texinfo.

gettext:
	$(SPHINXBUILD) -b gettext $(I18NSPHINXOPTS) $(BUILDDIR)/locale
	$(blankline)
	@echo Build finished.

changes:
	$(SPHINXBUILD) -b changes $(ALLSPHINXOPTS) $(BUILDDIR)/changes
	$(blankline)
	@echo Build finished.

linkcheck:
	$(SPHINXBUILD) -b linkcheck $(ALLSPHINXOPTS) $(BUILDDIR)/linkcheck
	$(blankline)
	@echo Link check complete; look for any errors in the above output or in $(BUILDDIR)/linkcheck/output.txt.

doctest:
	$(SPHINXBUILD) -b doctest $(ALLSPHINXOPTS) $(BUILDDIR)/doctest
	$(blankline)
	@echo Testing of doctests in the sources finished, look at the results in $(BUILDDIR)/doctest/output.txt.
