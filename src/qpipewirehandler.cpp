#include "qpipewirehandler.h"

/** Event processing structs **/

struct group {
    const char *type;
    uint32_t flag;
    uint32_t version;
    const void *events;
};

#define DECLARE_GROUP(lower_name, title_name, upper_name, event_data) \
static const struct group group##title_name = { \
    .type = PW_TYPE_INTERFACE_##title_name, \
    .flag = PWItemFlag::ItemType##title_name, \
    .version = PW_VERSION_##upper_name, \
    .events = event_data, \
};

#define DECLARE_GROUP_EVENTS(lower_name, upper_name, info_name, info_fn) \
static const struct pw_##lower_name##_events lower_name##Events = { \
    PW_VERSION_##upper_name, \
    .info_name = info_fn, \
};

DECLARE_GROUP_EVENTS(link, LINK, info, QPipewireHandler::eventInfoLink)
DECLARE_GROUP_EVENTS(node, NODE, info, QPipewireHandler::eventInfoNode)
DECLARE_GROUP_EVENTS(port, PORT, info, QPipewireHandler::eventInfoPort)
DECLARE_GROUP(link, Link, LINK, &linkEvents)
DECLARE_GROUP(node, Node, NODE, &nodeEvents)
DECLARE_GROUP(port, Port, PORT, &portEvents)

static const struct group *groups[] =
{
    &groupLink,
    &groupNode,
    &groupPort,
};

static const struct pw_registry_events registryEvents = {
    PW_VERSION_REGISTRY_EVENTS,
    .global = QPipewireHandler::eventRegistryGlobal,
    .global_remove = QPipewireHandler::eventRegistryGlobalRemove,
};

struct ItemData {
    uint32_t id;
    PWItem *item;
    QPipewireHandler *eventThread;
    const char *type;
    struct pw_proxy *proxy;
    struct spa_hook objectListener;
};


/** Public API **/


void QPipewireHandler::start(void)
{
    pw_init(nullptr, nullptr);

    m_loop = pw_thread_loop_new("QPipewire", NULL);
    m_context = pw_context_new(pw_thread_loop_get_loop(m_loop), NULL, 0);
    m_core = pw_context_connect(m_context, NULL, 0);
    m_registry = pw_core_get_registry(m_core, PW_VERSION_REGISTRY, 0);
    m_initComplete = false;
    m_lastData = nullptr;

    pw_registry_add_listener(m_registry, &m_registryListener,
            &registryEvents, this);
    pw_thread_loop_start(m_loop);
}

void QPipewireHandler::stop(void)
{
    pw_thread_loop_stop(m_loop);
    pw_thread_loop_destroy(m_loop);
}


/** Helper functions **/


static const struct group *findGroupFor(const char *type, uint32_t version)
{
    const struct group *result = nullptr;

    for (size_t i = 0;i < SPA_N_ELEMENTS(groups);i++) {
        if (strcmp(groups[i]->type, type) == 0 &&
            groups[i]->version <= version) {
            result = groups[i];
            break;
        }
    }

    return result;
}

static void addPropsToHash(QHash<QString, QString> *h,
        const struct spa_dict *props)
{
    const struct spa_dict_item *it;

    spa_dict_for_each(it, props) {
        QString key = QString(it->key);
        QString value = QString(it->value);

        h->insert(key, value);
    }
}

static QHash<QString, QString> hashifyProps(const struct spa_dict *props)
{
    QHash<QString, QString> result = QHash<QString, QString>();
    const struct spa_dict_item *it;

    spa_dict_for_each(it, props) {
        QString key = QString(it->key);
        QString value = QString(it->value);

        result.insert(key, value);
    }

    return result;
}

static PWItemFlags propToPortDirection(QHash<QString, QString> h,
        const char *key)
{
    QString str = h.value(key);
    PWItemFlags result = PWItemFlag::None;

    if (str != "") {
        if (str == "in")
            result = PWItemFlag::ItemInput;
        else if (str == "out")
            result = PWItemFlag::ItemOutput;
    }

    return result;
}

static PWItemFlags propToFlag(QHash<QString, QString> h, const char *key,
        PWItemFlags value)
{
    QString s = h.value(key);
    PWItemFlags result = PWItemFlag::None;

    if (s != "")
        result = value;

    return result;
}

static uint propToInt(const struct spa_dict *props,
        const char *key)
{
    const char *str = spa_dict_lookup(props, key);
    const uint result = (str ? uint(pw_properties_parse_int(str)) : 0);

    return result;
}

static uint propToInt(QHash<QString, QString> h, const char *key)
{
    return h.value(key, "").toInt(nullptr, 10);
}

static PWItemFlags flagsForMediaClass(QString s)
{
    struct media_class_pair {
        const char *name;
        PWItemFlags flags;
    };

    media_class_pair table[] = {
        { "Audio",  PWItemFlag::NodeAudio },
        { "Input",  PWItemFlag::ItemInput },
        { "Midi",   PWItemFlag::NodeMidi },
        { "Output", PWItemFlag::ItemOutput },
        { "Sink",   PWItemFlag::ItemInput | PWItemFlag::NodeSink },
        { "Source", PWItemFlag::ItemOutput | PWItemFlag::NodeSource },
        { "Video",  PWItemFlag::NodeVideo },
    };

    PWItemFlags result = PWItemFlag::None;
    QStringList sl = s.split("/");

    foreach (QString entry, sl) {
        for (size_t i = 0;i < SPA_N_ELEMENTS(table);i++) {
            media_class_pair pair = table[i];

            if (entry == pair.name) {
                result |= pair.flags;
                break;
            }
        }
    }

    return result;
}

PWPort *QPipewireHandler::idToPort(uint id) const
{
    return m_ports.value(id, nullptr);
}

PWNode *QPipewireHandler::idToNode(uint id) const
{
    return m_nodes.value(id, nullptr);
}


/** Event handling **/


void QPipewireHandler::eventRegistryGlobal(void *self_, uint32_t id,
        uint32_t permissions, const char *type, uint32_t version,
        const struct spa_dict *props)
{
    const struct group *g = findGroupFor(type, version);

    if (g == nullptr || g->events == NULL)
        return;

    QPipewireHandler *self = (QPipewireHandler *)self_;
    ItemData *data = new ItemData;

    data->eventThread = self;
    data->type = type;
    data->id = id;
    data->proxy = (struct pw_proxy *)pw_registry_bind(self->m_registry, id,
            type, g->version, 0);

    pw_proxy_add_object_listener(data->proxy, &data->objectListener,
            g->events, data);

    if (self->m_initComplete == false)
        self->m_lastData = (void *)data;
}

void QPipewireHandler::eventRegistryGlobalRemove(void *self_, uint32_t id)
{
    QPipewireHandler *self = (QPipewireHandler *)self_;
    emit self->evItemRemoved(id);
}

#define EVENT_INFO_HEADER \
ItemData *data = (ItemData *)data_; \
QPipewireHandler *self = data->eventThread; \
QHash<QString, QString> props = hashifyProps(info->props);

#define EVENT_INFO_FOOTER \
pw_proxy_destroy(data->proxy); \
 \
if (self->m_lastData == data) { \
    emit self->evInitComplete(); \
    self->m_lastData = nullptr; \
    self->m_initComplete = true; \
}

void QPipewireHandler::eventInfoPort(void *data_, const struct pw_port_info *info)
{
    EVENT_INFO_HEADER
    int serial = propToInt(props, PW_KEY_OBJECT_SERIAL);
    PWItemFlags f = PWItemFlag::ItemTypePort;

    f |= propToPortDirection(props, PW_KEY_PORT_DIRECTION);
    f |= propToFlag(props, PW_KEY_PORT_PHYSICAL, PWItemFlag::PortPhysical);
    f |= propToFlag(props, PW_KEY_PORT_TERMINAL, PWItemFlag::PortTerminal);
    f |= propToFlag(props, PW_KEY_PORT_MONITOR, PWItemFlag::PortMonitor);
    f |= propToFlag(props, PW_KEY_PORT_CONTROL, PWItemFlag::PortControl);

    PWPort *newPort = new PWPort;

    newPort->flags = f;
    newPort->id = data->id;
    newPort->serial = serial;
    newPort->props = props;
    self->m_ports[data->id] = newPort;
    emit self->evPortAdded(newPort);
    EVENT_INFO_FOOTER
}

void QPipewireHandler::eventInfoLink(void *data_, const struct pw_link_info *info)
{
    EVENT_INFO_HEADER
    const uint portOutId = propToInt(props, PW_KEY_LINK_OUTPUT_PORT);
    const uint inPortId = propToInt(props, PW_KEY_LINK_INPUT_PORT);
    const uint serial = propToInt(props, PW_KEY_OBJECT_SERIAL);
    PWPort *portOut = self->idToPort(portOutId);
    PWItemFlags f = PWItemFlag::ItemTypeLink;

    if (portOut == nullptr || portOut->isOutput() == false)
        return;

    PWPort *inPort = self->idToPort(inPortId);

    if (inPort == nullptr || inPort->isInput() == false)
        return;

    PWLink *newLink = new PWLink;

    newLink->id = data->id;
    newLink->serial = serial;
    newLink->props = props;
    newLink->inNodeId = propToInt(props, PW_KEY_LINK_INPUT_NODE);
    newLink->inPortId = inPortId;
    newLink->nodeOutId = propToInt(props, PW_KEY_LINK_OUTPUT_NODE);
    newLink->portOutId = portOutId;
    newLink->inNode = self->idToNode(newLink->inNodeId);
    newLink->nodeOut = self->idToNode(newLink->nodeOutId);

    if (newLink->inNode->flags & PWItemFlag::NodeSink)
        f |= PWItemFlag::LinkPlayback;
    else if (newLink->nodeOut->flags & (PWItemFlag::NodeSink | PWItemFlag::NodeSource))
        f |= PWItemFlag::LinkRecord;
    else
        f |= PWItemFlag::LinkOther;

    newLink->flags = f;
    emit self->evLinkAdded(newLink);
    EVENT_INFO_FOOTER
}

void QPipewireHandler::eventInfoNode(void *data_, const struct pw_node_info *info)
{
    EVENT_INFO_HEADER
    const uint serial = propToInt(props, PW_KEY_OBJECT_SERIAL);
    PWItemFlags f = PWItemFlag::ItemTypeNode;
    QString app = props.value(PW_KEY_APP_NAME);
    QString nodeName = props.value(PW_KEY_NODE_NAME);

    f |= flagsForMediaClass(props.value(PW_KEY_MEDIA_CLASS));

    if ((f & (PWItemFlag::ItemInput | PWItemFlag::ItemOutput)) == 0) {
        QString str = props.value(PW_KEY_MEDIA_CATEGORY);

        if (str != "") {
            const QString media_category(str);
            if (media_category.contains("Duplex"))
                f |= PWItemFlag::ItemDuplex;
        }
    }

    PWNode *newNode = new PWNode;

    newNode->flags = f;
    newNode->id = data->id;
    newNode->serial = serial;
    newNode->props = props;
    newNode->nodeName = nodeName;
    self->m_nodes[data->id] = newNode;
    emit self->evNodeAdded(newNode);
    EVENT_INFO_FOOTER
}
