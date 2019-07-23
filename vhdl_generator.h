#ifndef VHDL_GENERATOR_H
#define VHDL_GENERATOR_H
#include <QPair>
#include <QString>

class QSettings;
class NamingTemplate;
class VHDLGenerator
{
public:
    VHDLGenerator(const QString& chip_id,
                  const QString& chip_name,
                  int address_width,
                  int register_width,
                  const QString& config_file,
                  const QString& interface_template_path,
                  const QString& interface_package_template_path);
    ~VHDLGenerator();

    QString generate_interface_package() const;
    QString generate_interface() const;
    static void reset_success_flag();
    static bool success();

private:
    QPair<QString, QString> generate_interface_package_block(const QString& block_id,
                                             const QString& block_name,
                                             const QString& block_abbr,
                                             const QString& block_start_addr,
                                             const QSet<QString>& writable_reg_types) const;
    QString generate_register_address_definition(const QString& reg_name,
                                                 const QString& reg_address) const;
    QString generate_register_init_definition(const QString& reg_id,
                                              const QString& reg_name,
                                              const NamingTemplate& signal_naming) const;
    QString get_register_init_value(const QString& reg_id) const;
    QVector<QString> generate_interface_block(const QString& block_id,
                                     const QString& block_name,
                                     const QString& block_abbr,
                                     const QSet<QString>& writable_reg_types,
                                     const QSet<QString>& output_signal_types,
                                     const QHash<QString, QVector<QString> >& reg_id2page) const;
    QString generate_port_definition(const QString& sig_name,
                                      int sig_width,
                                      const QString& sig_type_id,
                                      const QSet<QString>& output_signal_types) const;
    QString generate_register_definition(const QString& reg_id,
                                         const QString& reg_name,
                                         const QHash<QString, QVector<QString> >& reg_id2page) const;
    QString generate_initializing_register(const QString& reg_id,
                                           const QString& reg_name,
                                           const QHash<QString, QVector<QString> >& reg_id2page) const;
    QString generate_writing_register(const QString& reg_id,
                                      const QString& reg_name,
                                      const QHash<QString, QVector<QString> >& reg_id2page) const;
    QString generate_reading_register(const QString& reg_name) const;
    QString generate_assigning_value_to_readonly_register(const QString& reg_id, const QString& reg_name, const NamingTemplate& signal_naming) const;
    QString generate_assigning_value_to_constrol_signal(const QString& sig_id,
                                                        const QString& sig_name,
                                                        int sig_width,
                                                        bool reset_signal,
                                                        const NamingTemplate& register_naming) const;
    QString generate_assigning_value_to_paged_register(const QString& reg_id, const QString& reg_name, const QHash<QString, QVector<QString> >& reg_id2page) const;
    QPair<QSet<QString>, QSet<QString> > get_register_rw_types() const;
    QPair<QSet<QString>, QSet<QString> > get_signal_inout_types(const QSet<QString>& readable_reg_types,
                                                                const QSet<QString>& writable_reg_types) const;
    QHash<QString, QVector<QString> > get_register_id2page() const;

    const QString chip_id_, chip_name_;
    int address_width_, register_width_;
    const QString interface_template_path_, interface_package_template_path_;
    QSettings* config_;
    static bool success_;
};



#endif // VHDL_GENERATOR_H
