#include <QStyledItemDelegate>
#include <QApplication>
#include <QAbstractListModel>
#include <QPainter>

#include "iconlistview.h"

class IconDelegate : public QStyledItemDelegate
{
public:
    explicit IconDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {}

    static int widthHint()
    {
        return iconSize + iconSpacing * 2;
    }

    static int heightHint(const QFontMetrics &fm)
    {
        return iconSize + fm.height() + iconSpacing * 3;
    }

private:
    void paint(QPainter *p, const QStyleOptionViewItem &opt,
               const QModelIndex &index) const
    {
        const QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));

        QStyle *style = QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &opt, p, opt.widget);

        if (!icon.isNull()) {
            QRect r = opt.rect;
            r.translate((r.width() - iconSize) / 2, iconSpacing);
            r.setSize(QSize(iconSize, iconSize));

            icon.paint(p, r);
        }

        p->drawText(opt.rect.adjusted(0, 0, 0, -iconSpacing), Qt::AlignHCenter | Qt::AlignBottom,
                    index.data().toString());
    }

    QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &/*index*/) const
    {
        Q_ASSERT(opt.widget != nullptr);
        return QSize(widthHint(), heightHint(opt.widget->fontMetrics()));
    }

private:
    static constexpr int iconSize = 64;
    static constexpr int iconSpacing = 4;
};

class IconListModel : public QAbstractListModel
{
public:
    explicit IconListModel(QObject *parent = 0) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const
    { Q_UNUSED(parent); return m_data.count(); }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (role == Qt::DecorationRole) {
            return m_data.at(index.row()).icon;
        }

        if (role == Qt::DisplayRole) {
            return m_data.at(index.row()).title;
        }

        return QVariant();
    }

    void appendItem(const QString &title, const QIcon &icon)
    {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_data << Data(title, icon);
        endInsertRows();
    }

private:
    struct Data {
        QString title;
        QIcon icon;

        Data() {}
        Data(const QString &atitle, const QIcon &aicon) : title(atitle), icon(aicon) {}
    };

private:
    QVector<Data> m_data;
};

IconListView::IconListView(QWidget *parent)
    : QListView(parent),
      m_model(new IconListModel(this))
{
    setModel(m_model);
    setItemDelegate(new IconDelegate(this));

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(selectionModel(), &QItemSelectionModel::currentRowChanged,
            [this](const QModelIndex &idx) {
                emit itemSelected(idx.row());
            }
    );
}

void IconListView::appendItem(const QString &title, const QIcon &icon)
{
    m_model->appendItem(title, icon);
}

int IconListView::selectedRow() const
{
    return selectionModel()->selectedRows().first().row();
}

void IconListView::selectRow(int row) const
{
    selectionModel()->setCurrentIndex(m_model->index(row), QItemSelectionModel::SelectCurrent);
}

void IconListView::prepareSize()
{
    int max = IconDelegate::widthHint();
    for (int row = 0; row < m_model->rowCount(); row++) {
        const QString title = m_model->data(m_model->index(row, 0), Qt::DisplayRole).toString();
        max = qMax(fontMetrics().width(title), max);
    }
    setFixedWidth(max * 1.2);

    setMinimumHeight(m_model->rowCount() * IconDelegate::heightHint(fontMetrics())
                     + fontMetrics().height() * 2);
}
