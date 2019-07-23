#ifndef NAMING_TEMPLATE_H
#define NAMING_TEMPLATE_H
#include <QHash>

class NamingTemplate
{
public:
    explicit NamingTemplate();
    NamingTemplate(const NamingTemplate& naming);
    NamingTemplate& operator=(const NamingTemplate& naming);
    void set_naming_template(const QString& naming_template);
    QString get_naming_template() const;
    QString get_extended_name(const QString& given_name) const;
    QString get_given_name(const QString& extended_name) const;
    void update_key(const QString& key, const QString& value);
    void clear();

private:
    QHash<QString, QString> key2value;
    QString naming_template;
};

#endif // NAMING_TEMPLATE_H
