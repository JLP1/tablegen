#!/bin/sh
#
# cgi program to test tt2ht2
#
echo "Content-type: text/html"
echo ""
( cat tablegen.top ; ls -l | head -10 ) | ./tablegen
echo "</table>"

echo "<p>"
echo "TableGen can create complex html markup beyond just tables, while associating patterned attributes to the elements including HTML tags, CSS tags, and Javascript. Hover over table fields to view Javascript UI behavior highlighting row and column crosshairs.  <br><p> <a href=\"../homework.html\"> Back to work! &raquo;</a> </p>"
echo "</p>"
echo "<table class=\"cal\">"
cal   | ./tablegen
echo "</table></body></html>"