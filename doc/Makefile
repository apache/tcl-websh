all: html/index.html

html/index.html: quickref.xml quickref.xsl
	xsltproc -o html/ quickref.xsl quickref.xml

#quickref.html: quickref.xml quickref.xsl
#	xsltproc -o quickref.html quickref.xsl quickref.xml

#quickref.txt: quickref.html
#	links -dump quickref.html > quickref.txt

clean:
	rm -f *.html html/*.html *.txt