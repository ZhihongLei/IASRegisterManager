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
        src=\"{MATHJAX_ROOT}/MathJax.js?config=TeX-AMS-MML_HTMLorMML-full\">\n\
    </script>\n\
</head>\n\
<body>\n\
{HTML} \n\
</body>\n\
</html>";

const static QString html_text_template = "{CONTENT}\n";

const static QString html_table_template = "<style>\n\
        table {border-top: 1px solid black; border-bottom: 1px solid black}\n\
        th {border-bottom: 1px solid black}\n\
        </style>\n\
        <center>\n\
        <table>\n\
         {TABLE}\n\
        </table>\n\
        </center>";

const static QString html_image_template = "<style>\n\
        figure { display: inline-block;}\n\
        figure img { vertical-align: top;}\n\
        figure figcaption { text-align: center;}\n\
        </style>\n\
        <center>\n\
        <figure>\n\
        <img style='width: 80%; object-fit: contain' src=\"{IMAGE}\" alt=\"{CAPTION}\"/>\n\
          <figcaption>{CAPTION}</figcaption>\n\
        </figure>\n\
        </center>";


const static QString DOC_DELIMITER = "%^%";
const static QString SPECIAL_CHARS = " !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

const static QString SIGNAL_NAMING_TEMPLATE = "{BLOCK_ABBR}_{NAME}_SIG";
const static QString REGISTER_NAMING_TEMPLATE = "{BLOCK_ABBR}_{NAME}_REG";

#endif // GLOBAL_VARIABLES_H
