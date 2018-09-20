/*****************************************************************************
 * Kbv DBus monitor
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2013.12.30
 ****************************************************************************/
#ifndef KbVDBUSMONITOR_H_
#define KbVDBUSMONITOR_H_
#include <QtCore>
#include <QDBusMetaType>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMessage>

struct   AYASV
{
  QByteArray  ay1;
  QList<QMap<QString, QByteArray> > asv;
};

/*******************************************************
 * Service   org.gtk.vfs.Daemon
 * interface org.gtk.vfs.MountTracker
 * signals   Mounted, Unmounted
 * signature sosssssbay(aya{sv})ay, name='mount'
 */
struct  kbvMountVfs  //mounts
 {
   QString          dbusId;         //dbus_id
   QDBusObjectPath  objectPath; //object_path
   QString          name;           //display_name
   QString          stableName;     //stable_name
   QString          xContentType;   //Since GVFS 1.0 only
   QString          iconName;       //icon
   QString          nameEncoding;   //prefered_filename_encoding
   QString          str;            //
   bool             userVisible;    //user_visible
   QByteArray       mountPoint;     //fuse_mountpoint
   struct AYASV     ayasv;
   QByteArray       ba;
 };
/* Interface name='org.gtk.vfs.MountTracker'
 * signal    ListMounts"
 * signature a(sossssssbay(aya{sv})ay)
 *
 * STRUCT mount_info
 * STRING           dbus_id
 * OBJECT_PATH      object_path
 * STRING           display_name
 * STRING           stable_name
 * STRING           x_content_types     Since GVFS 1.0 only !!!
 * STRING           icon
 * STRING           preferred_filename_encoding
 * BOOLEAN          user_visible
 * ARRAY BYTE       fuse_mountpoint
 * STRUCT           mount_spec
 *   ARRAY BYTE     mount_prefix
 *   ARRAY
 *     STRUCT       mount_spec_item
 *       STRING     key (server, share, type, user, host, port)
 *       ARRAY BYTE value
 * ARRAY BYTE       default_location	Since GVFS 1.5 only !!!*/

/*******************************************************
 * interface org.gtk.Private.RemoteVolumeMonitor
 * signals   'MountAdded, MountRemoved'
 * MOUNT_STRUCT_TYPE "(ssssssbsassa{sv})"
 * string               id
 * string               name
 * string               gicon_data
 * string               symbolic_gicon_data
 * string               uuid
 * string               root_uri
 * boolean              can-unmount
 * string               volume-id
 * array:string         x-content-types
 * string               sort_key
 * a{sv}                expansion
*/
struct  kbvRVMMountStruct    //Mount
 {
   QString    id;           //gduMount
   QString    name;         //volume label
   QString    iconNames;    //list of icon names
   QString    symbolicIcon; //
   QString    uuid;         //
   QString    mountPoint;   //mount point in file system
   bool       removable;    //
   QString    volumeId;     //
 };
/* signal  'List'
 * DRIVE_STRUCT_TYPE "(ssssbbbbbbbbuasa{ss}sa{sv})"
 *  string               id
 * string               name
 * string               gicon_data
 * string               symbolic_gicon_data
 * boolean              can-eject
 * boolean              can-poll-for-media
 * boolean              has-media
 * boolean              is-media-removable
 * boolean              is-media-check-automatic
 * boolean              can-start
 * boolean              can-start-degraded
 * boolean              can-stop
 * uint32               start-stop-type
 * array:string         volum
 * e-idsgduMount
 * dict:string->string  identifiers
 * string               sort_key
 * a{sv}                expansion
 * boolean              is-removable
 *
 * VOLUME_STRUCT_TYPE "(ssssssbbssa{ss}sa{sv})"
 * string               id
 * string               name
 * string               gicon_data
 * string               symbolic_gicon_data
 * string               uuid
 * string               activation_uri
 * boolean              can-mount
 * boolean              should-automount
 * string               drive-id
 * string               mount-id
 * dict:string->string  identifiers
 * string               sort_key
 * a{sv}                expansion
 * see>: gnome/gvfs .. gvfsproxyvolumemonitordaemon.c
 */

Q_DECLARE_METATYPE(kbvRVMMountStruct)
Q_DECLARE_METATYPE(QList<kbvRVMMountStruct>)
Q_DECLARE_METATYPE(kbvMountVfs)
Q_DECLARE_METATYPE(QList<kbvMountVfs>)

const QDBusArgument  &operator>>(const QDBusArgument &arg, kbvRVMMountStruct &smv);
QDBusArgument  &operator<<(QDBusArgument &arg, const kbvRVMMountStruct &smv);
const QDBusArgument   &operator>>(const QDBusArgument &arg, kbvMountVfs &smd);
QDBusArgument   &operator<<(QDBusArgument &arg, const kbvMountVfs &smd);

class KbvDBusMonitor : public QObject
{
  Q_OBJECT
  
public:
  KbvDBusMonitor(QObject *parent=nullptr);
  virtual ~KbvDBusMonitor();

signals:
void  mountedDevice(QStringList mountList);
void  unmountedDevice(QStringList mountList);

private slots:
void  vfsMountList(QDBusMessage msg);
void  vfsMounted(QDBusMessage msg);
void  vfsUnmounted(QDBusMessage msg);
void  privateRemVolMonList(QDBusMessage msg);
void  privateRemVolMonMounted(QDBusMessage msg);
void  privateRemVolMonUnmount(QDBusMessage msg);
void  mountError(QDBusError error);

private:
QDBusConnectionInterface  *dbusDaemon;
QDBusInterface            *mountedDevices, *mountedVolumes;
QDBusMessage              msgListMounts;

};
/* Services and interfaces (Ubuntu Gnome 16.04)
org.gtk.Private.UDisks2VolumeMonitor, org.gtk.Private.GduVolumeMonitor,
org.gtk.vfs.UDisks2VolumeMonitor, org.gtk.vfs.MTPVolumeMonitor
and org.gtk.vfs.GPhoto2VolumeMonitor
use commonly interface org.gtk.Private.RemoteVolumeMonitor:
Method List:
Received reply from :1.33, Arguments:
  [Argument: a(ssssbbbbbbbbuasa{ss}sa{sv}) {[Argument: (ssssbbbbbbbbuasa{ss}sa{sv}) "0x16384c0", "USB Flash Disk", ". GThemedIcon drive-removable-media-usb drive-removable-media drive-removable drive", ". GThemedIcon drive-removable-media-usb-symbolic drive-removable-media-symbolic drive-removable-symbolic drive-symbolic drive-removable-media-usb drive-removable-media drive-removable drive", true, false, true, true, true, false, false, false, 1, {"0x160ebd0"}, [Argument: a{ss} {"unix-device" = "/dev/sdc", "unix-device" = "/dev/sdc"}], "01hotplug/1468164554562231", [Argument: a{sv} {}]],
  [Argument: (ssssbbbbbbbbuasa{ss}sa{sv}) "0x16383c0", "Samsung SSD 840 EVO 250GB", ". GThemedIcon drive-harddisk-solidstate drive-harddisk drive", ". GThemedIcon drive-harddisk-solidstate-symbolic drive-harddisk-symbolic drive-symbolic drive-harddisk-solidstate drive-harddisk drive", false, false, true, false, true, false, false, false, 1, {}, [Argument: a{ss} {"unix-device" = "/dev/sda", "unix-device" = "/dev/sda"}], "00coldplug/00fixed/sd____a", [Argument: a{sv} {}]],
  [Argument: (ssssbbbbbbbbuasa{ss}sa{sv}) "0x1638340", "TOSHIBA MK5061GSYN", ". GThemedIcon drive-harddisk drive", ". GThemedIcon drive-harddisk-symbolic drive-symbolic drive-harddisk drive", false, false, true, false, true, false, false, false, 1, {"0x160eb30"}, [Argument: a{ss} {"unix-device" = "/dev/sdb", "unix-device" = "/dev/sdb"}], "00coldplug/00fixed/sd____b", [Argument: a{sv} {}]],
  [Argument: (ssssbbbbbbbbuasa{ss}sa{sv}) "0x16382c0", "TSSTcorp CDDVDW SH-224DB", ". GThemedIcon drive-optical drive", ". GThemedIcon drive-optical-symbolic drive-symbolic drive-optical drive", true, false, false, true, true, false, false, false, 1, {}, [Argument: a{ss} {"unix-device" = "/dev/sr0", "unix-device" = "/dev/sr0"}], "00coldplug/11removable/sr0", [Argument: a{sv} {}]]}],
  [Argument: a(ssssssbbssa{ss}sa{sv}) {[Argument: (ssssssbbssa{ss}sa{sv}) "0x160ebd0", "GT4", ". GThemedIcon drive-removable-media-usb drive-removable-media drive-removable drive", ". GThemedIcon drive-removable-media-usb-symbolic drive-removable-media-symbolic drive-removable-symbolic drive-symbolic drive-removable-media-usb drive-removable-media drive-removable drive", "", "", true, true, "0x16384c0", "0x161e810", [Argument: a{ss} {"class" = "device", "unix-device" = "/dev/sdc1", "label" = "GT4", "uuid" = "0836-53F7"}], "gvfs.time_detected_usec.1468164554811653", [Argument: a{sv} {}]],
  [Argument: (ssssssbbssa{ss}sa{sv}) "0x160eb30", "GT1", ". GThemedIcon drive-harddisk drive", ". GThemedIcon drive-harddisk-symbolic drive-symbolic drive-harddisk drive", "", "", true, false, "0x1638340", "0x161eb50", [Argument: a{ss} {"class" = "device", "unix-device" = "/dev/sdb1", "label" = "GT1", "uuid" = "c4f6fc96-78db-4c45-9a26-1dad6bbb7dbb"}], "gvfs.time_detected_usec.1468159742630468", [Argument: a{sv} {}]]}],
  [Argument: a(ssssssbsassa{sv}) {[Argument: (ssssssbsassa{sv}) "0x161e810", "GT4", ". GThemedIcon drive-removable-media-usb drive-removable-media drive-removable drive", ". GThemedIcon drive-removable-media-usb-symbolic drive-removable-media-symbolic drive-removable-symbolic drive-symbolic drive-removable-media-usb drive-removable-media drive-removable drive", "", "file:///media/gerd/GT4", true, "0x160ebd0", {}, "gvfs.time_detected_usec.1468164554852252", [Argument: a{sv} {}]],
  [Argument: (ssssssbsassa{sv}) "0x161eb50", "GT1", ". GThemedIcon drive-harddisk drive", ". GThemedIcon drive-harddisk-symbolic drive-symbolic drive-harddisk drive", "", "file:///media/GT1", true, "0x160eb30", {}, "gvfs.time_detected_usec.1468159742631778", [Argument: a{sv} {}]]}]
Member MountAdded:
  Received signal from :1.32, path /org/gtk/Private/RemoteVolumeMonitor, interface org.gtk.Private.RemoteVolumeMonitor, member MountAdded
  Arguments: "org.gtk.vfs.UDisks2VolumeMonitor", "0x20da9b0", [Argument: (ssssssbsassa{sv}) "0x20da9b0", "GT4",
  ". GThemedIcon drive-removable-media-usb drive-removable-media drive-removable drive", ". GThemedIcon drive-removable-media-usb-symbolic drive-removable-media-symbolic drive-removable-symbolic drive-symbolic drive-removable-media-usb drive-removable-media drive-removable drive",
  "", "file:///media/gerd/GT4", true, "0x20cabd0", {}, "gvfs.time_detected_usec.1468139030458684", [Argument: a{sv} {}]]
Member MountRemoved:
  Received signal from :1.32, path /org/gtk/Private/RemoteVolumeMonitor, interface org.gtk.Private.RemoteVolumeMonitor, member MountRemoved
  Arguments: "org.gtk.vfs.UDisks2VolumeMonitor", "0x20da9b0", [Argument: (ssssssbsassa{sv}) "0x20da9b0", "GT4",
  ". GThemedIcon drive-removable-media-usb drive-removable-media drive-removable drive", ". GThemedIcon drive-removable-media-usb-symbolic drive-removable-media-symbolic drive-removable-symbolic drive-symbolic drive-removable-media-usb drive-removable-media drive-removable drive",
  "", "file:///media/gerd/GT4", true, "", {}, "gvfs.time_detected_usec.1468139030458684", [Argument: a{sv} {}]]

org.gtk.vfs.Daemon
Method ListMounts:
  Arguments: [Argument: a(sossssssbay(aya{sv})ay) {[Argument: (sossssssbay(aya{sv})ay) ":1.78",
  [ObjectPath: /org/gtk/vfs/mount/1], "Netzwerk", "network:", "",
  ". GThemedIcon network-workgroup network", ". GThemedIcon network-workgroup-symbolic network-symbolic network-workgroup network", "", false, {0},
  [Argument: (aya{sv}) {47, 0}, [Argument: a{sv} {"type" = [Variant(QByteArray): {110, 101, 116, 119, 111, 114, 107, 0}]}]], {0}], [Argument: (sossssssbay(aya{sv})ay) ":1.82", [ObjectPath: /org/gtk/vfs/mount/1], "Dns-SD", "dns-sd:host=local", "", ". GThemedIcon network-workgroup network", ". GThemedIcon network-workgroup-symbolic network-symbolic network-workgroup network", "", false, {0}, [Argument: (aya{sv}) {47, 0}, [Argument: a{sv} {"host" = [Variant(QByteArray): {108, 111, 99, 97, 108, 0}], "type" = [Variant(QByteArray): {100, 110, 115, 45, 115, 100, 0}]}]], {0}], [Argument: (sossssssbay(aya{sv})ay) ":1.66", [ObjectPath: /org/gtk/vfs/mount/1], "Brennen", "burn:", "", "computer", ". GThemedIcon computer-symbolic computer", "", false, {0}, [Argument: (aya{sv}) {47, 0}, [Argument: a{sv} {"type" = [Variant(QByteArray): {98, 117, 114, 110, 0}]}]], {0}], [Argument: (sossssssbay(aya{sv})ay) ":1.59", [ObjectPath: /org/gtk/vfs/mount/1], "Papierkorb", "trash:", "", ". GThemedIcon user-trash user", ". GThemedIcon user-trash-symbolic user-symbolic user-trash user", "", false, {0}, [Argument: (aya{sv}) {47, 0}, [Argument: a{sv} {"type" = [Variant(QByteArray): {116, 114, 97, 115, 104, 0}]}]], {0}]}]
Member Mounted:
  Received signal from :1.2, path /org/gtk/vfs/mounttracker, interface org.gtk.vfs.MountTracker, member Mounted
  Arguments: [Argument: (sossssssbay(aya{sv})ay) ":1.80", [ObjectPath: /org/gtk/vfs/mount/1], "Canon Digital Camera",
  "gphoto2:host=%5Busb%3A001%2C010%5D", "x-content/image-dcf", ". GThemedIcon camera-photo camera", ". GThemedIcon camera-photo-symbolic camera-symbolic camera-photo camera", "", true,
  {47, 114, 117, 110, 47, 117, 115, 101, 114, 47, 49, 48, 48, 49, 47, 103, 118, 102, 115, 47, 103, 112, 104, 111, 116, 111, 50, 58, 104, 111, 115, 116, 61, 37, 53, 66, 117, 115, 98, 37, 51, 65, 48, 48, 49, 37, 50, 67, 48, 49, 48, 37, 53, 68, 0},
  [Argument: (aya{sv}) {47, 0}, [Argument: a{sv} {"host" = [Variant(QByteArray): {91, 117, 115, 98, 58, 48, 48, 49, 44, 48, 49, 48, 93, 0}], "type" = [Variant(QByteArray): {103, 112, 104, 111, 116, 111, 50, 0}]}]], {0}]
Member Unmounted:
  Received signal from :1.2, path /org/gtk/vfs/mounttracker, interface org.gtk.vfs.MountTracker, member Unmounted
  Arguments: [Argument: (sossssssbay(aya{sv})ay) ":1.82", [ObjectPath: /org/gtk/vfs/mount/1], "Canon Digital Camera",
  "gphoto2:host=%5Busb%3A001%2C011%5D", "x-content/image-dcf", ". GThemedIcon camera-photo camera", ". GThemedIcon camera-photo-symbolic camera-symbolic camera-photo camera", "", true,
  {47, 114, 117, 110, 47, 117, 115, 101, 114, 47, 49, 48, 48, 49, 47, 103, 118, 102, 115, 47, 103, 112, 104, 111, 116, 111, 50, 58, 104, 111, 115, 116, 61, 37, 53, 66, 117, 115, 98, 37, 51, 65, 48, 48, 49, 37, 50, 67, 48, 49, 49, 37, 53, 68, 0},
  [Argument: (aya{sv}) {47, 0}, [Argument: a{sv} {"host" = [Variant(QByteArray): {91, 117, 115, 98, 58, 48, 48, 49, 44, 48, 49, 49, 93, 0}], "type" = [Variant(QByteArray): {103, 112, 104, 111, 116, 111, 50, 0}]}]], {0}]

*/
/* D-Bus interfaces and arguments ********************************************
 * Common infos about d-bus arguments
 * The following argument types are supported for D-Bus methods (with respective closest types in GLib):
 *  b: boolean (gboolean, 0=false, 1=true, other values invalid)
 *  y: 8-bit unsigned integer (guint8)
 *  q/n: 16-bit unsigned/signed integer (guint16/gint16)
 *  u/i: 32-bit unsigned/signed integer (guint32/gint32)
 *  t/x: 64-bit unsigned/signed integer (guint64/gint64)
 *  d: IEEE 754 double precision floating point number (gdouble)
 *  s: UTF-8 encoded text string with NUL termination (only one NUL allowed) (gchar* with additional restrictions)
 *  o: Object_path: must begin with an ASCII '/' character and only contain the ASCII characters "[A-Z][a-z][0-9]_"
 *  g: Signature: zero or more single complete types
 *  a: Array of the following type specification (array of strinQ_DECLARE_METATYPE(kbvMountVolume)
 *  gs as, array of structs with integers a(ii))
 *     array of array if integers aai
 *  (): Struct: e.g. struct containing two integers (ii) or nested structs (ii(iii))
 *  v: variant
 *  {}: dictionary: it occurs only as an array element type; it has exactly two single complete types inside the
 *      curly braces; the first single complete type (the "key") must be a basic (e.g. a{sv}). 
 *  See the official D-Bus documentation http://dbus.freedesktop.org/doc/dbus-specification.html.
 *
 * Scalar types (type-strings 'b', 'y', 'n', 'q', 'i', 'u', 'x', 't' and 'd') ),
 * strings (type-strings 's', 'ay', 'o' and 'g') and
 * arrays of string (type-strings 'as', 'ao' and 'aay')
 * are mapped to the natural types, e.g. gboolean, gdouble, gint, gchar*, gchar** and so on.
 * Everything else is mapped to the GVariant type.
 *
*/
#endif // KbVDBUSMONITOR_H
/****************************************************************************/
