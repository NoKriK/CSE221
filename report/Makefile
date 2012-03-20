# By Yann LEGER

NAME		=	system-measurement-project
VERSION		=	1.0
DATE		=	$(shell date +%Y%m%d)
FNAME		=	${NAME}-${DATE}-v${VERSION}.pdf
TARGETDIR	=	../target/
SRC		=	${NAME}.tex

LATEXCC		=	pdflatex
LATEXFLAGS	=	-output-directory ${TARGETDIR}

${FNAME}:	premake
		cd src/ && ${LATEXCC} ${LATEXFLAGS} ${SRC}
		cp target/${NAME}.pdf ${FNAME}

all: ${FNAME}

open	:	${FNAME}
		open ${FNAME}

premake:
		cd src/ && mkdir -p ${TARGETDIR}
