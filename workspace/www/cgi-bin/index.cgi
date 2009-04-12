#!/usr/bin/perl
#安装说明:
#复制程序代码,并存成env.cgi
#以ascii模式上传至主机cgi-bin目录后将属性改成755
$|=1;
print "Content-type: text/html\n\n";
print "<html><head><title>环境变量清单</title>\n";
print "</head>\n";
print "<body>\n";
print "<table border=1>\n";
print "<tr>\n";
print "<td>变量名称</td>\n";
print "<td>目前状况</td>\n";
print "</tr>\n";
foreach $fieldname(keys %ENV){
print "<tr>\n";
print "<td>$fieldname</td>\n";
print "<td>$ENV{$fieldname}</td>\n";
print "</tr>\n\n";
}
print "</table>\n";
print "</body></html>\n";
exit;
