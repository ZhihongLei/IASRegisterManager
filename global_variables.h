#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H
#include "naming_template.h"

class NamingTemplate;
extern QString MATHJAX_ROOT;
extern QString HTML_TEMPLATE, HTML_TEXT_TEMPLATE, HTML_IMAGE_TEMPLATE, HTML_TABLE_TEMPLATE;
extern QString RECENT_REGISTER_ID;
extern QString RESOURCES_BASE_DIR;
extern QString LOG_PATH;

const static QString DEFAULT_HTML_TEMPLATE = "\
        <html>\n\
        <head>\n\
            <script type=\"text/x-mathjax-config\">\n\
                MathJax.Hub.Config({tex2jax: {inlineMath: [['$','$'], ['\\\\(', '\\\\)']]}});\n\
            </script>\n\
            <script type=\"text/javascript\"\n\
                src=\"${MATHJAX_ROOT}/MathJax.js?config=TeX-AMS-MML_HTMLorMML-full\">\n\
            </script>\n\
        </head>\n\
        <body>\n\
        ${HTML} \n\
        </body>\n\
        </html>";
const static QString DEFAULT_HTML_TEXT_TEMPLATE = "${CONTENT}<br>\n";
const static QString DEFAULT_HTML_IMAGE_TEMPLATE = "<style>\n\
                figure img {display: block; margin-left: auto; margin-right: auto;}\n\
                figure figcaption { text-align: center;}\n\
                </style>\n\
                <center>\n\
                <figure>\n\
                ${CAPTION_TOP}\n\
                <img style='width: ${WIDTH}; object-fit: contain' src=\"${IMAGE}\" alt=\"${CAPTION}\"/>\n\
                  ${CAPTION_BOTTOM}\n\
                </figure>\n\
                </center>";
const static QString DEFAULT_HTML_TABLE_TEMPLATE = "<style>\n\
        table {border-top: 1px solid black; border-bottom: 1px solid black}\n\
        th {border-bottom: 1px solid black}\n\
        caption#tab {caption-side: ${CAPTION_POS}}\n\
        </style>\n\
        <center>\n\
        <table>\n\
         ${TABLE}\n\
        </table>\n\
        </center>";

const static QString DOC_DELIMITER = "%^%";
const static QString SPECIAL_CHARS = " !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

const static QString DEFAULT_REGISTER_NAMING_TEMPLATE = "${BLOCK_ABBR}_${GIVEN_NAME}_REG";
const static QString DEFAULT_SIGNAL_NAMING_TEMPLATE = "${BLOCK_ABBR}_${GIVEN_NAME}";

extern NamingTemplate GLOBAL_REGISTER_NAMING;
extern NamingTemplate GLOBAL_SIGNAL_NAMING;

#endif // GLOBAL_VARIABLES_H
