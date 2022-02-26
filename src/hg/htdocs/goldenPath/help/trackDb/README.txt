To add a new statement to the trackDb documentation:

1. Add your statement documentation to trackDbLibrary.shtml

2. Add references to your statement to trackDbDoc.html

3. If intended for hubs:
        * Add to the latest trackDbHub doc (e.g. trackDbHub.v1.html), with support level 'new'.
                (see intro of the doc for explanation of support levels).
        * Use hubCheck to verify the doc entry is properly formatted:
            % make
            % hubCheck -settings \
                -version=https://hgwdev-<user>/goldenPath/help/trackDb/trackDbHub.v1.html
          OR
            % make alpha
            % hubCheck -settings -test
          
- See the long comment at the top of trackDbLibrary.shtml for details


HOW TO VERSION:
When to update versions:
  The consensus is to be conservative and only rollover the trackDbHub.v#.html
  version with a  major feature addition, or deprecation of an existing feature.
  It was suggested that the driver of versioning should really be the needs of the
  hub user and hub-display community.

What to do to update:
  0. You will probably be in a situation where you have gone past the moment (say v4 when on v3).
  In this case first copy what you have that you think will be v4 from your current version.
  For example, "cp trackDbHub.v3.html trackDbHub.v4.html".  Also edit the file to say (v4) in the text.
  Then you can go back a step by doing a "git checkout abadcafe-last-commit-before-newstuff-efefg133 trackDbHub.v3.html"
  This will return the v3 file to where it was before the new changes were introduced.

  1. In the makefile in this directory you will need to update
  CURRENT_HUB_SPEC=v#old  to be CURRENT_HUB_SPEC=v#new  (change v1 to v2)
  and change HUB_SPEC_FILES= to include the new trackDbHub.v#.html
  update goldenPath/help/trackDb/makefile
      +               trackDbHub.vPrev#.html \
      +               trackDbHub.vNew#.html
  This is an easy step to FORGET, please remember to add the new file in the makefile.

  2. Update trackDbHub.html to point to the new html file.
  <!--#include virtual="trackDbHub.v#.html"-->

  3. From that point forward the previous version remains unchanged.

How to push:
  To release changes it will be easier if you push the trackDbLibrary.shtml (note .shtml)
  and the current version of the file trackDbHub.v#.html and trackDbHub.html which designates
  the current version.

      Example Push Request:

        Please push the following files:

        /usr/local/apache/htdocs/goldenPath/help/trackDb/trackDbDoc.html
        /usr/local/apache/htdocs/goldenPath/help/trackDb/trackDbHub.html
        /usr/local/apache/htdocs/goldenPath/help/trackDb/trackDbLibrary.shtml
        /usr/local/apache/htdocs/goldenPath/help/trackDb/trackDbHub.v1.html

        from hgwbeta --> RR.

        Reason: Push of trackDbLibrary.shtml changes and associated trackDbHub.html page
        that currently points to trackDbHub.v1.html

        Thank you!


