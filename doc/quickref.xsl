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

  <xsl:import href="/usr/local/share/xsl/docbook/docbook/html/docbook.xsl" />

  <xsl:variable name="arg.choice.opt.open.str">?</xsl:variable>
  <xsl:variable name="arg.choice.opt.close.str">?</xsl:variable>
  <xsl:variable name="arg.choice.req.open.str"></xsl:variable>
  <xsl:variable name="arg.choice.req.close.str"></xsl:variable>

  <xsl:variable name="arg.choice.def.open.str"></xsl:variable>
  <xsl:variable name="arg.choice.def.close.str"></xsl:variable>

  <!-- <u> isn't a valid tag, so the best way to do this is via CSS,
  but that's a pain - fixme-later.-->
  <xsl:template name="inline.underlineseq">
    <xsl:param name="content">
      <xsl:call-template name="anchor"/>
      <xsl:apply-templates/>
    </xsl:param>
    <u><xsl:copy-of select="$content"/></u>
  </xsl:template>

  <xsl:template match="optional">
    <xsl:value-of select="$arg.choice.opt.open.str"/>
    <xsl:call-template name="inline.charseq"/>
    <xsl:value-of select="$arg.choice.opt.close.str"/>
  </xsl:template>

  <xsl:template match="option">
    <xsl:call-template name="inline.underlineseq"/>
  </xsl:template>

  <xsl:template match="group|arg">
    <xsl:variable name="choice" select="@choice"/>
    <xsl:variable name="rep" select="@rep"/>
    <xsl:variable name="sepchar">
      <xsl:choose>
	<xsl:when test="ancestor-or-self::*/@sepchar">
	  <xsl:value-of select="ancestor-or-self::*/@sepchar"/>
	</xsl:when>
	<xsl:otherwise>
	  <xsl:text> </xsl:text>
	</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:if test="position()>1"><xsl:value-of select="$sepchar"/></xsl:if>
    <xsl:choose>
      <xsl:when test="$choice='plain'">
	<xsl:value-of select="$arg.choice.plain.open.str"/>
      </xsl:when>
      <xsl:when test="$choice='req'">
	<xsl:value-of select="$arg.choice.req.open.str"/>
      </xsl:when>
      <xsl:when test="$choice='opt'">
	<xsl:value-of select="$arg.choice.opt.open.str"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:value-of select="$arg.choice.def.open.str"/>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:choose>
      <xsl:when test="$choice='plain'">
	<xsl:call-template name="inline.monoseq"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:call-template name="inline.underlineseq"/>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:choose>
      <xsl:when test="$rep='repeat'">
	<xsl:value-of select="$arg.rep.repeat.str"/>
      </xsl:when>
      <xsl:when test="$rep='norepeat'">
	<xsl:value-of select="$arg.rep.norepeat.str"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:value-of select="$arg.rep.def.str"/>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:choose>
      <xsl:when test="$choice='plain'">
	<xsl:value-of select="$arg.choice.plain.close.str"/>
      </xsl:when>
      <xsl:when test="$choice='req'">
	<xsl:value-of select="$arg.choice.req.close.str"/>
      </xsl:when>
      <xsl:when test="$choice='opt'">
	<xsl:value-of select="$arg.choice.opt.close.str"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:value-of select="$arg.choice.def.close.str"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>



</xsl:stylesheet>
