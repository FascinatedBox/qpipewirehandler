#ifndef QPIPEWIREHANDLER_H
# define QPIPEWIREHANDLER_H
# include <pipewire/pipewire.h>

# include <QHash>
# include <QMap>
# include <QObject>
# include <QString>

enum PWItemFlag {
    None         = 0,

    ItemInput    = 0x00001,
    ItemOutput   = 0x00002,
    ItemDuplex   = 0x00001 | 0x00002,

    ItemTypeNode = 0x00004,
    ItemTypeLink = 0x00008,
    ItemTypePort = 0x00010,

    LinkOther    = 0x00020,
    LinkPlayback = 0x00040,
    LinkRecord   = 0x00080,

    NodeAudio    = 0x00100,
    NodeMidi     = 0x00200,
    NodeSink     = 0x00400,
    NodeSource   = 0x00800,
    NodeVideo    = 0x01000,
    NodeVirtual  = 0x02000,

    PortControl  = 0x04000,
    PortMonitor  = 0x08000,
    PortPhysical = 0x10000,
    PortTerminal = 0x20000,
};

Q_DECLARE_FLAGS(PWItemFlags, PWItemFlag);
Q_DECLARE_OPERATORS_FOR_FLAGS(PWItemFlags)
# define IS_FN(name, flag) \
bool is##name(void) { return flags & PWItemFlag::flag; }

struct PWItem {
    PWItemFlags flags;
    int id;
    int serial;
    QHash<QString, QString> props;

    IS_FN(Input, ItemInput)
    IS_FN(Output, ItemOutput)
};

struct PWNode : PWItem {
    QString nodeName;

    IS_FN(Sink, NodeSink)
    IS_FN(Source, NodeSource)
};

struct PWLink : PWItem {
    uint inNodeId;
    uint inPortId;
    uint nodeOutId;
    uint portOutId;
    PWNode *inNode;
    PWNode *nodeOut;
};

struct PWPort : PWItem {};

# undef IS_FN

class QPipewireHandler : public QObject
{
    Q_OBJECT

public:
    QPipewireHandler(void) {}

    static void eventRegistryGlobal(void *self_, uint32_t id,
            uint32_t permissions, const char *type, uint32_t version,
            const struct spa_dict *props);
    static void eventRegistryGlobalRemove(void *self_, uint32_t id);
    static void eventInfoLink(void *data_, const struct pw_link_info *info);
    static void eventInfoNode(void *data_, const struct pw_node_info *info);
    static void eventInfoPort(void *data_, const struct pw_port_info *info);

    void start(void);
    void stop(void);

signals:
    void evInitComplete(void);
    void evItemRemoved(uint);
    void evLinkAdded(PWLink *);
    void evNodeAdded(PWNode *);
    void evPortAdded(PWPort *);

private:
    PWNode *idToNode(uint id) const;
    PWPort *idToPort(uint id) const;

    struct pw_context *m_context;
    struct pw_core *m_core;
    struct pw_registry *m_registry;
    struct pw_thread_loop *m_loop;
    struct spa_hook m_coreListener;
    struct spa_hook m_registryListener;
    void *m_lastData;
    bool m_initComplete;

    QMap<int, PWNode *> m_nodes;
    QMap<int, PWPort *> m_ports;
};

#endif
