#include "naming_template.h"

NamingTemplate::NamingTemplate()
{

}

NamingTemplate::NamingTemplate(const NamingTemplate& naming)
{
    set_naming_template(naming.get_naming_template());
    for (const QString& key : naming.key2value.keys())
        update_key(key, naming.key2value[key]);
}

NamingTemplate& NamingTemplate::operator=(const NamingTemplate& naming)
{
    set_naming_template(naming.get_naming_template());
    for (const QString& key : naming.key2value.keys())
        update_key(key, naming.key2value[key]);
    return *this;
}

QString NamingTemplate::get_extended_name(const QString &given_name) const
{
    QString naming = naming_template;
    for (const QString& key : key2value.keys()) naming.replace(key, key2value[key]);
    naming.replace("${GIVEN_NAME}", given_name);
    return naming;
}

QString NamingTemplate::get_given_name(const QString &extended_name) const
{
    QString ref = get_extended_name("");
    int i = 0;
    while (i < ref.size() && ref[i] == extended_name[i]) i++;
    int j = 0, m = ref.size() - 1, n = extended_name.size() - 1;
    while (j < ref.size() && ref[m - j] == extended_name[n - j]) j++;
    return extended_name.mid(i, n - i - j + 1);
}

void NamingTemplate::set_naming_template(const QString &naming_template)
{
    NamingTemplate::naming_template = naming_template;
}

QString NamingTemplate::get_naming_template() const
{
    return naming_template;
}

void NamingTemplate::update_key(const QString &key, const QString &value)
{
    key2value[key] = value;
}

void NamingTemplate::clear()
{
    naming_template.clear();
    key2value.clear();
}
