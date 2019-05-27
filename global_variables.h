#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H
#include <QString>

extern QString gDatabase, gDBHost, gUser, gPassword;
extern QString mathjax_root;
const static QString html_template = "\
<html>\n\
<head>\n\
    <script type=\"text/x-mathjax-config\">\n\
        MathJax.Hub.Config({tex2jax: {inlineMath: [['$','$'], ['\\(','\\)']]}});\n\
    </script>\n\
    <script type=\"text/javascript\"\n\
        src=\"{MATHJAX_ROOT}/MathJax.js?config=TeX-AMS-MML_HTMLorMML\">\n\
    </script>\n\
</head>\n\
<body>\n\
{CONTENT} \n\
</body>\n\
</html>";

const static QString DOC_TABLE_DELIMITER = "%^%";
const static QString SPECIAL_CHARS = " !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

#endif // GLOBAL_VARIABLES_H
