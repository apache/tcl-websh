<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:doc="http://nwalsh.com/xsl/documentation/1.0"
  exclude-result-prefixes="doc"
  version='1.0'>

  <!--
  <xsl:output method="html"
  encoding="ISO-8859-1"
  indent="no"/>
  -->

  <!-- ********************************************************************
  $Id$
  ********************************************************************

  This file is part of the XSL DocBook Stylesheet distribution.
  See ../README or http://nwalsh.com/docbook/xsl/ for copyright
  and other information.

  ******************************************************************** -->

  <!-- ==================================================================== -->

  <xsl:import
    href="http://docbook.sourceforge.net/release/xsl/1.68.1/html/chunk.xsl"
  />
  <!-- local alternatives to the network URL for faster processing 
  <xsl:import
    href="/usr/share/sgml/docbook/stylesheet/xsl/nwalsh/html/chunk.xsl"
  />
  <xsl:import
    href="/usr/local/share/xsl/docbook/docbook/html/chunk.xsl"
  /> 
  -->

  <xsl:param name="use.id.as.filename" select="1"/>

  <xsl:variable name="arg.choice.opt.open.str" xml:space="preserve">?</xsl:variable>
  <xsl:variable name="arg.choice.opt.close.str">?</xsl:variable>

  <xsl:variable name="arg.choice.req.open.str" xml:space="preserve"></xsl:variable>
  <xsl:variable name="arg.choice.req.close.str"></xsl:variable>

  <xsl:variable name="arg.choice.def.open.str" xml:space="preserve"></xsl:variable>
  <xsl:variable name="arg.choice.def.close.str"></xsl:variable>

  <xsl:template name="inline.underlineseq">
    <xsl:param name="content">
      <xsl:call-template name="anchor"/>
      <xsl:apply-templates/>
    </xsl:param>
      <xsl:copy-of select="$content"/>
  </xsl:template>

  <xsl:template name="inline.monounderlineseq">
    <xsl:param name="content">
      <xsl:call-template name="anchor"/>
      <xsl:apply-templates/>
    </xsl:param>
    <tt><xsl:copy-of
	  select="$content"/></tt>
  </xsl:template>


  <xsl:template match="/article/section/title" mode="titlepage.mode"
    priority="2">
    <hr/>
    <xsl:call-template name="section.title"/>
  </xsl:template>

  <xsl:template match="para">
    <xsl:variable name="p">
      <p style="width:90%">
	<xsl:if test="position() = 1 and parent::listitem">
	  <xsl:call-template name="anchor">
	    <xsl:with-param name="node" select="parent::listitem"/>
	  </xsl:call-template>
	</xsl:if>
	<xsl:call-template name="anchor"/>
	<xsl:apply-templates/>
      </p>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="$html.cleanup != 0">
	<xsl:call-template name="unwrap.p">
	  <xsl:with-param name="p" select="$p"/>
	</xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
	<xsl:copy-of select="$p"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="optional">
    <xsl:value-of select="$arg.choice.opt.open.str"/>
    <xsl:call-template name="inline.charseq"/>
    <xsl:value-of select="$arg.choice.opt.close.str"/>
  </xsl:template>

  <xsl:template match="option">
    <xsl:value-of select="$arg.choice.req.open.str"/>
    <xsl:call-template name="inline.monounderlineseq"/>
    <xsl:value-of select="$arg.choice.req.close.str"/>
  </xsl:template>

  <xsl:template match="command">
    <span style="font-family:monospace">
      <xsl:call-template name="inline.boldseq"/>
    </span>
  </xsl:template>

  <xsl:template match="cmdsynopsis">
    <div class="{name(.)}">
      <span style="background:#bbbbff">
	<xsl:call-template name="anchor"/>
	<xsl:apply-templates/>
      </span>
    </div>
  </xsl:template>

  <xsl:template match="cmdsynopsis/command">
    <span style="font-weight:bold">
      <xsl:call-template name="inline.monoseq"/>
    </span>
    <xsl:text> </xsl:text>
  </xsl:template>

  <xsl:template match="varlistentry">
    <dt>
      <xsl:call-template name="anchor"/>
      <span style="background:#bbbbff">
	<xsl:apply-templates select="term"/>
      </span>
    </dt>
    <dd>
      <div style="padding:4 ; margin-top:3 ;
	margin-bottom:3 ; width:75%" >
	<xsl:apply-templates select="listitem"/>
      </div>
    </dd>
  </xsl:template>

  <xsl:template match="listitem/para">
    <div style="margin-bottom:6">
      <xsl:apply-templates/>
    </div>
  </xsl:template>

  <xsl:template match="programlisting|screen|synopsis">
    <xsl:param name="suppress-numbers" select="'0'"/>
    <xsl:variable name="id">
      <xsl:call-template name="object.id"/>
    </xsl:variable>

    <xsl:call-template name="anchor"/>

    <xsl:variable name="content">
      <xsl:choose>
	<xsl:when test="$suppress-numbers = '0'
	  and @linenumbering = 'numbered'
	  and $use.extensions != '0'
	  and $linenumbering.extension != '0'">
	  <xsl:variable name="rtf">
	    <xsl:apply-templates/>
	  </xsl:variable>
	  <pre class="{name(.)}">
	    <xsl:call-template name="number.rtf.lines">
	      <xsl:with-param name="rtf" select="$rtf"/>
	    </xsl:call-template>
	  </pre>
	</xsl:when>
	<xsl:otherwise>
	  <pre style="background:#bbffbb ; width:75%"
	    class="{name(.)}">
	    <xsl:apply-templates/>
	  </pre>
	</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="$shade.verbatim != 0">
	<table xsl:use-attribute-sets="shade.verbatim.style">
	  <tr>
	    <td>
	      <xsl:copy-of select="$content"/>
	    </td>
	  </tr>
	</table>
      </xsl:when>
      <xsl:otherwise>
	<xsl:copy-of select="$content"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


</xsl:stylesheet>

