pdflatex = pdflatex -interaction=errorstopmode -halt-on-error -shell-escape

all: definition.pdf

collected.tex: parse.py syntax.ebnf
	python parse.py collected > collected.tex

ordered.tex: parse.py syntax.ebnf
	python parse.py ordered > ordered.tex

keywords.tex: parse.py syntax.ebnf
	python parse.py keywords > keywords.tex

definition.pdf: definition.tex ../preamble.tex collected.tex ordered.tex keywords.tex refs.bib 
	$(pdflatex) definition
	bibtex definition
	$(pdflatex) definition
	$(pdflatex) definition

clean:
	rm -f \
    *.aux \
    *.log \
    *.pdf \
    collected.tex \
    ordered.tex \
    keywords.tex

