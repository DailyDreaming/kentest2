"\n"
"<ul>\n"
"<li><a href=\"#About\">About</a></li>\n"
"<li><a href=\"#Privacy\">Privacy</a></li>\n"
"<li><a href=\"#Quickstart\">Quickstart</a></li>\n"
"<li><a href=\"#Account\">Account</a></li>\n"
"<li><a href=\"#Finding\">Finding data</a></li>\n"
"<li><a href=\"#Downloading\">Downloading data</a></li>\n"
"<li><a href=\"#Definitions\">Definitions</a></li>\n"
"</ul>\n"
"\n"
"<!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n"
"# Break section\n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -->\n"
"<hr />\n"
"<a name=\"About\"></a>\n"
"<h3>About</h3>\n"
"\n"
"<p><span style=\"color:#0066CC; font-weight:bold;\">The CIRM Data Warehouse</span></p>\n"
"\n"
"<p>The stem cell hub is a data warehouse for stem cell genomic files. It houses primary\n"
"data files such as DNA reads in fastq format, as well as many types of files derived\n"
"from mapping and other analysis of the primary data, and PDF and other document\n"
"files describing protocols. It has a small but flexible system for associating metadata\n"
"tags with a file. Any CIRM-genomic associated lab can submit data. Once submitted\n"
"data is treated as prepublication human sequence data, and access is only allowed to\n"
"authorized users.</p>\n"
"\n"
"<a name=\"Wranglers\"></a>\n"
"<p><span style=\"color:#0066CC; font-weight:bold;\">Data wranglers</span></p>\n"
"The data wranglers assist labs with bringing data into the CIRM Data Warehouse and with downloading submitted data.<br/><br/>\n"
"Clay Fischer &lt;<span style=\"color:#0066CC\"><em>clmfisch@ucsc.edu</em></span>&gt;<br />\n"
"Parisa Nejad &lt;<span style=\"color:#0066CC\"><em>pnejad@ucsc.edu</em></span>&gt;<br />\n"
"Nick Fong &lt;<span style=\"color:#0066CC\"><em>nfong@ucsc.edu</em></span>&gt;\n"
"\n"
"<br /><br/>\n"
"<strong>Engineers</strong><br/>\n"
"You can contact the engineers with information about data pipelines and submission summary pages.<br/><br/>\n"
"Galt Barber &lt;<span style=\"color:#0066CC\"><em>galt@soe.ucsc.edu</em></span>&gt;<br/><br/>\n"
"\n"
"\n"
"<!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n"
"# Break section\n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -->\n"
"<hr />\n"
"<a name=\"Privacy\"></a>\n"
"<h3>Privacy</h3>\n"
"\n"
"<span style=\"color:#0066CC; font-weight:bold;\">Data is private between labs</span>\n"
"<p>Currently, <strong>most data is restricted</strong>. This means the data is only accessible by members of the lab which submitted it. If you wish to view restricted data for your lab, contact a wrangler to be granted access. </p>\n"
"\n"
"<span style=\"color:#0066CC; font-weight:bold;\">Data is private from the public</span>\n"
"<p>Currently, data is protected from the public with use of a VPN. This requires setting up an account with our system administrators (<em>cluster-admin@soe.ucsc.edu</em>).</p>\n"
"<p>An SSL certificate is in the process of being set up, adding HTTPS encryption to your cirm-01 account. At That point we will remove the VPN requirement. When tumor maps are integrated, they will use your account authentication as well if dealing with restricted data.</p>\n"
"\n"
"<!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n"
"# Break section\n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -->\n"
"<hr />\n"
"<a name=\"Quickstart\"></a>\n"
"<h3>Quickstart</h3>\n"
"<p><span style=\"color:#0066CC; font-weight:bold;\">Login or create an account</span></p>\n"
"<p>Login to the website. If you don't have an account, you can make one.</a>\n"
"<p><span style=\"color:#0066CC; font-weight:bold;\">Browse files</span></p>\n"
"<p>At the top navigation, click the browse button and go to <a href=\"http://cirm-01/cgi-bin/cdwWebBrowse?cdwCommand=browseFiles&cdwFile_filter=\">Files</a>:</p>\n"
"<img src=\"../images/cdwImages/navigation.png\" alt=\"Top navigation should be displayed\" />\n"
"<p>This will bring you to a list of files.</p>\n"
"<img src=\"../images/cdwImages/files.png\" alt=\"file browser should be displayed\" />\n"
"<br />\n"
"<p><span style=\"color:#0066CC; font-weight:bold;\">Filter results</span></p>\n"
"<p>If have not used filters, the filters will be blank, as shown above. You can edit the cells and press enter to filter your results.</p>\n"
"<p>When your cursor is in the cell, hit the down arrow on your keyboard to show a drop-down list of every item you can filter to.</p>\n"
"<img src=\"../images/cdwImages/dropdown.png\" alt=\"file browser should be displayed\" />\n"
"<p>Filters take advantage of basic UNIX wildcard syntax. It looks for an exact match unless asterisks are used:</p>\n"
"<img src=\"../images/cdwImages/filtered.png\" alt=\"file browser should be displayed\" />\n"
"\n"
"<!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n"
"# Break section\n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -->\n"
"<hr />\n"
"<a name=\"Account\"></a>\n"
"<h3>Account (CDW website)</h3>\n"
"<p>The top navigation bar will have the option to <a href=\"http://cirm-01/cgi-bin/hgLogin?hgLogin.do.displayLoginPage=1\">login</a>:</p>\n"
"\n"
"<img src=\"../images/cdwImages/loginScreen.png\" alt=\"login screen should be displayed\" />\n"
"\n"
"<p>If you do not currently have an account, at the bottom you will see an option to create an account.</p>\n"
"\n"
"<img src=\"../images/cdwImages/createAccount.png\" alt=\"account creation screen should be displayed\" />\n"
"\n"
"<p>To view restricted data, contact a <a href=\"#Wranglers\">data wrangler</a> and they will be able to assist with granting access to your data.</p>\n"
"\n"
"<h3>Account (VPN)</h3>\n"
"<p>Data is restricted from the public with the use of a VPN, currently. </p>\n"
"<p>It is assumed you have VPN access already if you are reading this, but you may need to set it up for other members in your lab.</p>\n"
"<p>Contact a <a href=\"#Wranglers\">data wrangler</a> and they can coordinate a time with you and our system administators to set up your VPN account over the phone.</p>\n"
"<p>You will need to install <a href=\"https://tunnelblick.net/\" target=\"_blank\">Tunnelblick</a> before your call.</p>\n"
"\n"
"<h3>Account (Server)</h3>\n"
"<p> We no longer grant command line access to our  server and are in the process of removing accounts.</p>\n"
"\n"
"<h3>Account (Tumor Map)</h3>\n"
"This feature is currently being implemented. It will share authentication with your CDW website account.\n"
"\n"
"<!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n"
"# Break section\n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -->\n"
"<hr />\n"
"<a name=\"Finding Data\"></a>\n"
"<h3>Finding data</h3>\n"
"\n"
"\n"
"<p><span style=\"color:#0066CC; font-weight:bold;\">Files</span></p>\n"
"Under ‘browse’ go to Files. Click into the text box of a column and hit the drop-down. This shows everything for that column which you have the authorization to view. You can filter the columns down with the select list, or typing what you want with a wildcard. For example:\n"
"\n"
"Under sample_label, try *liver* and you’ll get any data sets with sample_labels.\n"
"\n"
"<p><span style=\"color:#0066CC; font-weight:bold;\">Tracks</span></p>\n"
"\n"
"<p><span style=\"color:#0066CC; font-weight:bold;\">Tags</span></p>\n"
"\n"
"<p><span style=\"color:#0066CC; font-weight:bold;\">Data sets</span></p>\n"
"<p>This contains descriptions of the data sets submitted, and summaries of the submission including vizualizations.</p>\n"
"\n"
"<p><span style=\"color:#0066CC; font-weight:bold;\">Query</span></p>\n"
"http://cirm-01/cgi-bin/cdwWebBrowse?cdwCommand=query\n"
"\n"
"<!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n"
"# Break section\n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -->\n"
"<hr />\n"
"<a name=\"Downloading\"></a>\n"
"<h3>Downloading</h3>\n"
"<p>Note to labs: your data may only be obtained from the CIRM Data Warehouse website. It cannot be downloaded until complete sequence data and metadata are available.</p>\n"
"<p><span style=\"color:#0066CC; font-weight:bold;\">One file</span></p>\n"
"\n"
"<p><span style=\"color:#0066CC; font-weight:bold;\">Batch downloads</span></p>\n"
"\n"
"<!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n"
"# Break section\n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -->\n"
"<hr />\n"
"<a name=\"Submitting\"></a>\n"
"<h3>Definitions</h3>\n"
"<p><span style=\"color:#0066CC; font-weight:bold;\">Help submitting data</span></p>\n"
"<p></p>\n"
"\n"
"\n"
"\n"
"<!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n"
"# Break section\n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -->\n"
"<hr />\n"
"<h3>Deprecated</h3>\n"
"This being a prototype, there's not much help available.  Try clicking and hovering over the Browse link on the top bar to expose a menu. The trickiest part of the system is the query link.The query link has you type in a SQL-like query. Try 'select * from files where accession' to get all metadata tags from files that have passed basic format validations. Instead of '*' you could use a comma separated list of tag names. Instead of 'accession' you could put in a boolean expression involving field names and constants. String constants need to be surrounded by quotes - either single or double.\n"
"\n"
"<br><br>\n"
