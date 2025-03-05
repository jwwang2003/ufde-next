<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html"/>

<xsl:template match="report">
  <html>
  <body>
  	<div>---------------------------------------------------------------------------------------------------------------------------------------------------------------------</div>
  	<div><font size="6" face="Calibri"><b>Static Timing Analyzer Report</b></font></div>
  	<div><font size="3" face="Calibri"><i>Copyright of Fudan University. All rights reserved.</i></font></div>
  	<br/><br/>
  	<table width="100%">
  		<tr>
  			<td width="25%"><b><font size="4" face="Courier New">Design File:</font></b></td>
  			<td width="75%"><font size="4" face = "Calibri"><xsl:value-of select="@design"/></font></td>
  		</tr>
  		<tr>
  			<td width="25%"><b><font size="4" face="Courier New">Device:</font></b></td>
  			<td width="75%"><font size="4" face = "Calibri"><xsl:value-of select="@device"/></font></td>
  		</tr>
  	</table>
  	<div>---------------------------------------------------------------------------------------------------------------------------------------------------------------------</div>
  	<br/>
  	<xsl:for-each select="part">
  		<div>---------------------------------------------------------------------------------------------------------------------------------------------------------------------</div>
  		<div><font size="4" face="Calibri"><b>
  			<xsl:if test="@mode='R2R'"> Register to Register Information </xsl:if>
  			<xsl:if test="@mode='R2O'"> Register to Output Information </xsl:if>
  			<xsl:if test="@mode='I2R'"> Input to Register Information </xsl:if>
  		</b></font></div>
  		<div>---------------------------------------------------------------------------------------------------------------------------------------------------------------------</div>
  		<br/><br/>
  		<xsl:for-each select="domain">
  			<p>
  				---------------------------------------------------------------------
  				<font size="4" face="Calibri">
  					<b><xsl:value-of select="@clk_net"/></b>
  				</font>
  				----------------------------------------------------------------------
  			</p>
  			
  			<br/>
  			<xsl:for-each select="section">
  				<p>
  				----------------------------------------------------
  				<font size="4" face="Calibri">
  					<b>Type: <xsl:value-of select="@type"/></b>
  				</font>
  				----------------------------------------------------
  			</p>
  				<xsl:for-each select="path">
  					<p>
  						<font size="4" face = "Calibri">
  							<b>Data Path <xsl:value-of select="@id"/></b>
  						</font>
  					</p>
  					<table width="100%">
  						<tr>
  							<td width="5%"/>
  							<td width="15%"><font size="3" face = "Courier New">Source:</font></td>
  							<td width="5%"><font size="3" face = "Courier New"><xsl:value-of select="@src"/></font></td>
  							<td width="5%"><font size="3" face = "Courier New"><xsl:value-of select="@src_module"/></font></td>
  							<td width="70%"/>
  						</tr>
  						<tr>
  							<td width="5%"/>
  							<td width="15%"><font size="3" face = "Courier New">Destination:</font></td>
  							<td width="5%"><font size="3" face = "Courier New"><xsl:value-of select="@dst"/></font></td>
  							<td width="5%"><font size="3" face = "Courier New"><xsl:value-of select="@dst_module"/></font></td>
  							<td width="70%"/>
  						</tr>
  					</table>
  						
  					<table width="100%">
  						<tr>
  							<td width="10%"/>
  							<td width="10%" align="center"><font size="4" face = "Courier New"><b>Pins:</b></font></td>
  							<td width="55%"></td>
  							<td width="10%"><font size="4" face = "Courier New"><b>AT</b></font></td>
  							<td width="15%"/>
  						</tr>
  						<tr>
  							<td width="10%"/>
  							<td width="10%" align="center">---------</td>
  							<td width="55%"></td>
  							<td width="10%">--------</td>
  							<td width="15%"/>
  						</tr>
  						
  						<xsl:for-each select="node">
  							<tr>
  								<td width="10%"/>
  								<td width="10%" align="center"><font size="3" face = "Courier New"><xsl:value-of select="@dir"/></font></td>
  								<td width="55%"><font size="3" face = "Courier New"><xsl:value-of select="@name"/></font></td>
  								<td width="10%"><font size="3" face = "Courier New"><xsl:value-of select="@tarr"/></font></td>
  								<td width="15%"/>
  								
  							</tr>
  						</xsl:for-each>
  						
  						<tr>
  							<td width="10%"/>
  							<td width="10%"></td>
  							<td width="55%"></td>
  							<td width="10%">--------</td>
  							<td width="15%"/>
  						</tr>
  					</table>
  					<table width="100%">
  						<tr>
  							<td width="10%"/>
  							<td width="10%"></td>
  							<td width="45%"></td>
  							<td width="10%"><b><font size="3" face = "Courier New">Slack:</font></b></td>
  							<td width="10%"><font size="3" face = "Courier New"><xsl:value-of select="@slack"/></font></td>
  							<td width="15%"/>
  						</tr>
  					</table>
  					
  					<br/><br/>
  				</xsl:for-each>		
  			</xsl:for-each>	
  			<p>---------------------------------------------------------------------------------------------------------------------------------------------------------------------</p>
  		</xsl:for-each>
  		<br/>
  		
  		<table width="100%">
  			<tr>
  				<td width="62%"><font size = "4" face = "Calibri"><b>Minimum Period: </b></font></td>
  				<td>
  					<font size="3" face = "Courier New">
  						<xsl:value-of select="@min_period"/> ns
  					</font>
  				</td>
  			</tr>
  			<tr>
  				<td width="62%"><font size = "4" face = "Calibri"><b>Maximum Frequency: </b></font></td>
  				<td>
  					<font size="3" face = "Courier New">
  						<xsl:value-of select="@max_freq"/> MHz
  					</font>
  				</td>
  			</tr>
  		</table>
  	</xsl:for-each>
  </body>
  </html>
</xsl:template>

</xsl:stylesheet>