all: html/index.html

html/index.html: quickref.xml quickref.xsl
	mkdir -p html
	xsltproc -o html/ quickref.xsl quickref.xml

check:
	nsgmls -s -wxml quickref.xml

clean:
	rm -f *.html html/*.html *.txt
	rmdir html
