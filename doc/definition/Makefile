pdflatex = pdflatex -interaction=errorstopmode -halt-on-error -shell-escape

all: once

collected.tex: parse.py syntax.ebnf
	python parse.py collected > collected.tex

ordered.tex: parse.py syntax.ebnf
	python parse.py ordered > ordered.tex

keywords.tex: parse.py syntax.ebnf
	python parse.py keywords > keywords.tex

full: definition.tex preamble.tex collected.tex ordered.tex keywords.tex refs.bib 
	$(pdflatex) definition
	bibtex definition
	$(pdflatex) definition
	$(pdflatex) definition

once: definition.tex preamble.tex collected.tex ordered.tex keywords.tex refs.bib 
	$(pdflatex) definition

clean:
	rm -f \
    *.aux \
    *.log \
    *.pdf \
    collected.tex \
    ordered.tex \
    keywords.tex

