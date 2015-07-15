<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
    xmlns:d="http://docbook.org/ns/docbook"
    xmlns:fo="http://www.w3.org/1999/XSL/Format"
    version="1.0">
    <xsl:import href="profile-docbook.xsl"/>
    <!-- Apply XHLTHL extension. -->
    <xsl:import href="highlight.xsl"/>
    <xsl:import href="../oxygen_custom.xsl"/>
    <xsl:attribute-set name="monospace.verbatim.properties">
        <xsl:attribute name="font-size">9pt</xsl:attribute>
        <!-- <xsl:attribute name="keep-together.within-column">always</xsl:attribute>  -->
    </xsl:attribute-set>
    <xsl:attribute-set name="shade.verbatim.style">
        <xsl:attribute name="background-color">#F0F0F0</xsl:attribute>
        <xsl:attribute name="border-width">0.5pt</xsl:attribute>
        <xsl:attribute name="border-style">solid</xsl:attribute>
        <xsl:attribute name="border-color">#575757</xsl:attribute>
        <xsl:attribute name="padding">3pt</xsl:attribute>
    </xsl:attribute-set>
</xsl:stylesheet>