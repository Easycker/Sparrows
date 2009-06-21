#!/usr/bin/perl
$|=1;
print "Content-type: text/html\n\n";
print "<html><head><title>环境变量清单</title>\n";
print "</head>\n";
print "<body>\n";
print "<table border=1>\n";
print "<tr>\n";
print "<td>变量名称</td>\n";
sleep 10;
print "<td>目前状况</td>\n";
print "</tr>\n";
foreach $fieldname(keys %ENV){
print "<tr>\n";
print "<td>$fieldname</td>\n";
sleep 10;
print "<td>$ENV{$fieldname}</td>\n";
print "</tr>\n\n";
}
print "</table>\n";
print "</body></html>\n";
exit;
