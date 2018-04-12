LATEX	= latex -shell-escape
BIBTEX	= bibtex
DVIPS	= dvips
DVIPDF  = dvipdft
XDVI	= xdvi -gamma 4
GH		= gv

EXAMPLES = $(wildcard *.c)
SRC	:= $(shell egrep -l '^[^%]*\\begin\{document\}' *.tex)
TRG	= $(SRC:%.tex=%.dvi)
PSF	= $(SRC:%.tex=%.ps)
PDF	= $(SRC:%.tex=%.pdf)

pdf: $(PDF)

ps: $(PSF)

$(TRG): %.dvi: %.tex $(EXAMPLES)
	$(LATEX) $<
	$(BIBTEX) $(<:%.tex=%)
	$(LATEX) $<
	$(LATEX) $<

$(PSF):%.ps: %.dvi
	$(DVIPS) -R -Poutline -t letter $< -o $@

$(PDF): %.pdf: %.ps
	ps2pdf $<

default: all

all: pdf clean

clean:
	rm -f *.ps *.dvi *.out *.log *.aux *.bbl *.blg *.pyg

.PHONY: all show clean ps pdf showps