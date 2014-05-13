pdflatex = pdflatex -interaction=errorstopmode -halt-on-error -shell-escape

all: definition.pdf

definition.pdf: definition.tex ../preamble.tex
	$(pdflatex) definition

clean:
	rm -f \
    *.aux \
    *.log \
    *.pdf

