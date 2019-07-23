#ifndef DOCUMENT_GENERATOR_H
#define DOCUMENT_GENERATOR_H

#include <QVector>
#include <QHash>

class Authenticator;
class NamingTemplate;
class DocumentGenerator
{
public:
    enum CAPTION_POSITION {TOP, BOTTOM};
    DocumentGenerator(const QString& chip_id,
                      const QString& chip_name,
                      int address_width,
                      int register_width,
                      bool msb_first,
                      const QString& user_id,
                      Authenticator* authenticator,
                      const CAPTION_POSITION& img_pos=CAPTION_POSITION::BOTTOM,
                      const CAPTION_POSITION& tab_pos=CAPTION_POSITION::TOP,
                      const QString& show_paged_register="control_signal");


    static void add_doc_type(const QString& doc_type);
    static void reset_success_flag();
    static bool success();

    static QString generate_html(const QString& doc_type, const QString& content,
                                 const CAPTION_POSITION& img_pos=CAPTION_POSITION::BOTTOM,
                                 const CAPTION_POSITION& tab_pos=CAPTION_POSITION::TOP);
    static QString generate_text_html(const QString& text);
    static QString generate_image_html(const QString& caption, const QString& width, const QString& path, const CAPTION_POSITION& pos=CAPTION_POSITION::BOTTOM);
    static QString generate_table_html(const QString& caption, const QVector<QVector<QString> >& cells, const CAPTION_POSITION& pos=CAPTION_POSITION::TOP);

    static QString generate_tex(const QString& doc_type, const QString& content,
                                const CAPTION_POSITION& img_pos=CAPTION_POSITION::BOTTOM,
                                const CAPTION_POSITION& tab_pos=CAPTION_POSITION::TOP);
    static QString generate_text_tex(const QString& text);
    static QString generate_image_tex(const QString& caption, const QString& width, const QString& path, const CAPTION_POSITION& pos=CAPTION_POSITION::BOTTOM);
    static QString generate_table_tex(const QString& caption, const QVector<QVector<QString> >& cells, const CAPTION_POSITION& pos=CAPTION_POSITION::TOP);

    QString generate_html_document();
    QString generate_tex_document();
    QString generate_chip_level_html_document();
    QString generate_chip_level_tex_document();
    QString generate_block_level_html_document(const QString& block_id,
                                           const QString& block_name,
                                           const QString& block_abbr,
                                           const QString& block_start_addr,
                                           const QHash<QString, QVector<QString> >& reg_id2page);
    QString generate_block_level_tex_document(const QString& block_id,
                                           const QString& block_name,
                                           const QString& block_abbr,
                                           const QString& block_start_addr,
                                           const QHash<QString, QVector<QString> >& reg_id2page);

    QString generate_register_bit_table_html(const QVector<QVector<QString> >& signal_items, const NamingTemplate& signal_naming);
    QString generate_register_bit_table_tex(const QVector<QVector<QString> >& signal_items, const NamingTemplate& signal_naming);
    QString generate_register_signal_bullets_html(const QVector<QString>& signal_ids, const QVector<QString>& signal_names);
    QString generate_register_signal_bullets_tex(const QVector<QString>& signal_ids, const QVector<QString>& signal_names);
    QString generate_register_level_html_document(const QString& reg_id, const QString& reg_name, const QString& address, const NamingTemplate& signal_naming, const QHash<QString, QVector<QString> >& reg_id2page);
    QString generate_register_level_tex_document(const QString& reg_id, const QString& reg_name, const QString& address, const NamingTemplate& signal_naming, const QHash<QString, QVector<QString> >& reg_id2page);
    QHash<QString, QVector<QString> > get_register_id2page() const;

private:
    static QString pure_text_to_tex(const QString& text);
    static QString get_previous_k_chars(int i, int k, const QString& text);
    static QString generate_doc(const QString& doc_type,
                                const QString& content,
                                QString (*f_text)(const QString& text),
                                QString (*f_img) (const QString& caption, const QString& width, const QString& path, const CAPTION_POSITION& pos),
                                QString (*f_tab) (const QString& caption, const QVector<QVector<QString> >& cells, const CAPTION_POSITION& pos),
                                const CAPTION_POSITION& img_pos=CAPTION_POSITION::BOTTOM,
                                const CAPTION_POSITION& tab_pos=CAPTION_POSITION::TOP);

    static QVector<QString> doc_types_;
    const QString chip_name_, chip_id_, user_id_;
    const int register_width_, address_width_;
    const bool msb_first_;
    Authenticator *authenticator_;
    const CAPTION_POSITION image_cap_pos_, table_cap_pos_;
    const QString show_paged_register_;
    static bool success_;
    QHash<QString, QString> seen_signal2reg_;
};

#endif // DOCUMENT_GENERATOR_H
