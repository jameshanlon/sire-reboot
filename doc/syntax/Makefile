pdflatex = pdflatex -interaction=stoponerror -file-line-error -shell-escape

all: syntax.pdf

collected.tex: syntax.ebnf
	python parse.py collected > collected.tex

ordered.tex: syntax.ebnf
	python parse.py ordered > ordered.tex

keywords.tex: syntax.ebnf
	python parse.py keywords > keywords.tex

syntax.pdf: syntax.tex collected.tex ordered.tex keywords.tex
	$(pdflatex) syntax.tex

clean:
	rm -f \
    *.aux \
    *.log \
    *.pdf \
    collected.tex \
    ordered.tex \
    keywords.tex

