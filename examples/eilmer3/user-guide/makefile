# makefile for eilmer3-user-guide and e3march-user-guide
# PJ, March 2007, June 2014

eilmer3-user-guide.pdf: eilmer3-user-guide.tex \
		axi-cylinder.tex \
		back-nozzle.tex \
		bar-476.tex \
		bianca-epfl.tex \
		blunt-wedge.tex \
		chem.tex \
		cone20-simple.tex \
		cone20-udf.tex \
		finite-cylinder.tex \
		gas_models.tex \
		hayabusa.tex \
		inject.tex \
		lehr-sphere.tex \
		mms-euler.tex \
		mnm-implosion-problem.tex \
		n90.tex \
		odw.tex \
		paraview.tex \
		periodic-shear-layer.tex \
		sawada-sphere.tex \
		scram1.tex \
		sharp.tex \
		sharp-pyfun.tex \
		simple-ramp.tex \
		sod-3d.tex \
		sphere-cone.tex \
		sphere-heat-transfer.tex \
		therm-exchange.tex \
		turbo-sc10.tex \
		turbo-sc10-parametric.tex \
		udf.tex \
		vortex.tex
	export BIBINPUTS=$HOME/papers/bibtex/
	pdflatex eilmer3-user-guide
	- bibtex eilmer3-user-guide
	pdflatex eilmer3-user-guide
	makeindex -l eilmer3-user-guide
	pdflatex eilmer3-user-guide
	pdflatex eilmer3-user-guide

e3march-user-guide.pdf: e3march-user-guide.tex
	export BIBINPUTS=$HOME/papers/bibtex/
	pdflatex e3march-user-guide
	- bibtex e3march-user-guide
	pdflatex e3march-user-guide
	makeindex -l e3march-user-guide
	pdflatex e3march-user-guide
	pdflatex e3march-user-guide

clean:
	- rm eilmer3-user-guide.log e3march-user-guide.log \
		*.aux \
		eilmer3-user-guide.idx e3march-user-guide.idx \
		eilmer3-user-guide.ilg e3march-user-guide.ilg \
		eilmer3-user-guide.ind e3march-user-guide.ind \
		eilmer3-user-guide.bbl e3march-user-guide.bbl \
		eilmer3-user-guide.blg e3march-user-guide.blg \
		eilmer3-user-guide.out e3march-user-guide.out \
		eilmer3-user-guide.toc e3march-user-guide.toc

all_clean:
	make clean
	- rm eilmer3-user-guide.pdf e3march-user-guide.pdf
