/*****************************************************************************
 * Kbv DBus monitor
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2013.12.30
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvDBusMonitor.h"
#include <QDBusReply>
#include <QDBusConnectionInterface>


KbvDBusMonitor::KbvDBusMonitor(QObject *parent) : QObject(parent=nullptr)
{
  QList<QVariant> args;
  
  qDBusRegisterMetaType<kbvRVMMountStruct>();
  qDBusRegisterMetaType<QList<kbvRVMMountStruct> >();
  qDBusRegisterMetaType<kbvMountVfs>();
  qDBusRegisterMetaType<QList<kbvMountVfs> >();

  QDBusConnection kbvSessionBus = QDBusConnection::sessionBus();
  dbusDaemon = kbvSessionBus.interface();
  
  
  //When service vfs.Daemon is registered:
  //connect to signals "ListMounts, Mounted, Unmounted" for devices
  //Between ubuntu 12.04LTS and 14.04LTS the interface call was changed. The old interface
  //with signals "listMounts, mounted, unmounted" get catched in the error slot
  if(dbusDaemon->isServiceRegistered("org.gtk.vfs.Daemon").value())
    {
      //Create an interface to vfsdaemon and get already mounted devices by gvfs mountTracker
      mountedDevices = new QDBusInterface("org.gtk.vfs.Daemon", "/org/gtk/vfs/mounttracker",
                                             "org.gtk.vfs.MountTracker", kbvSessionBus, this);
      mountedDevices->callWithCallback("ListMounts", args, this, SLOT(vfsMountList(QDBusMessage)), SLOT(mountError(QDBusError)));
      kbvSessionBus.connect("org.gtk.vfs.Daemon", "/org/gtk/vfs/mounttracker", "org.gtk.vfs.MountTracker",
                            "Mounted", this, SLOT(vfsMounted(QDBusMessage)));
      kbvSessionBus.connect("org.gtk.vfs.Daemon", "/org/gtk/vfs/mounttracker", "org.gtk.vfs.MountTracker",
                            "Unmounted", this, SLOT(vfsUnmounted(QDBusMessage)));
      //qDebug() << "KbvDBusMonitor service org.gtk.vfs.Daemon interface vfs.MountTracker"; //###########
    }

  //When service GduVolumeMonitor is registered (excludes UDisk2VolumeMonitor)
  //connect to signals "List, MountAdded, MountedRemoved" for volumes
  if(dbusDaemon->isServiceRegistered("org.gtk.Private.GduVolumeMonitor").value())
    {
      //Establish interface org.gtk.Private.RemoteVolumeMonitor for service org.gtk.Private.GduVolumeMonitor
      mountedVolumes = new QDBusInterface("org.gtk.Private.GduVolumeMonitor", "/org/gtk/Private/RemoteVolumeMonitor",
                                          "org.gtk.Private.RemoteVolumeMonitor", kbvSessionBus, this);
      mountedVolumes->callWithCallback("List", args, this, SLOT(privateRemVolMonList(QDBusMessage)), SLOT(mountError(QDBusError)));
      kbvSessionBus.connect("org.gtk.Private.GduVolumeMonitor", "/org/gtk/Private/RemoteVolumeMonitor", "org.gtk.Private.RemoteVolumeMonitor",
                            "MountAdded", this, SLOT(privateRemVolMonMounted(QDBusMessage)));
      kbvSessionBus.connect("org.gtk.Private.GduVolumeMonitor", "/org/gtk/Private/RemoteVolumeMonitor", "org.gtk.Private.RemoteVolumeMonitor",
                            "MountRemoved", this, SLOT(privateRemVolMonUnmounted(QDBusMessage)));
      //qDebug() << "KbvDBusMonitor service org.gtk.Private.GduVolumeMonitor interface org.gtk.Private.RemoteVolumeMonitor"; //###########
    }
  
  //Establish interface org.gtk.Private.RemoteVolumeMonitor for service org.gtk.Private.UDisks2VolumeMonitor
  //connect to signals "List, MountAdded, MountedRemoved" for volumes
  if(dbusDaemon->isServiceRegistered("org.gtk.Private.UDisks2VolumeMonitor").value())
    {
      mountedVolumes = new QDBusInterface("org.gtk.Private.UDisks2VolumeMonitor", "/org/gtk/Private/RemoteVolumeMonitor",
                                          "org.gtk.Private.RemoteVolumeMonitor", kbvSessionBus, this);
      mountedVolumes->callWithCallback("List", args, this, SLOT(privateRemVolMonList(QDBusMessage)), SLOT(mountError(QDBusError)));
      kbvSessionBus.connect("org.gtk.Private.UDisks2VolumeMonitor", "/org/gtk/Private/RemoteVolumeMonitor", "org.gtk.Private.RemoteVolumeMonitor",
                            "MountAdded", this, SLOT(privateRemVolMonMounted(QDBusMessage)));
      kbvSessionBus.connect("org.gtk.Private.UDisks2VolumeMonitor", "/org/gtk/Private/RemoteVolumeMonitor", "org.gtk.Private.RemoteVolumeMonitor",
                            "MountRemoved", this, SLOT(privateRemVolMonUnmount(QDBusMessage)));
      //qDebug() << "KbvDBusMonitor service org.gtk.Private.UDisks2VolumeMonitor interface org.gtk.Private.RemoteVolumeMonitor"; //###########
    }

  //Establish interface org.gtk.Private.RemoteVolumeMonitor for service org.gtk.vfs.UDisks2VolumeMonitor
  //connect to signals "List, MountAdded, MountedRemoved" for volumes
  if(dbusDaemon->isServiceRegistered("org.gtk.vfs.UDisks2VolumeMonitor").value())
    {
      mountedVolumes = new QDBusInterface("org.gtk.vfs.UDisks2VolumeMonitor", "/org/gtk/Private/RemoteVolumeMonitor",
                                          "org.gtk.Private.RemoteVolumeMonitor", kbvSessionBus, this);
      mountedVolumes->callWithCallback("List", args, this, SLOT(privateRemVolMonList(QDBusMessage)), SLOT(mountError(QDBusError)));
      kbvSessionBus.connect("org.gtk.vfs.UDisks2VolumeMonitor", "/org/gtk/Private/RemoteVolumeMonitor", "org.gtk.Private.RemoteVolumeMonitor",
                            "MountAdded", this, SLOT(privateRemVolMonMounted(QDBusMessage)));
      kbvSessionBus.connect("org.gtk.vfs.UDisks2VolumeMonitor", "/org/gtk/Private/RemoteVolumeMonitor", "org.gtk.Private.RemoteVolumeMonitor",
                            "MountRemoved", this, SLOT(privateRemVolMonUnmount(QDBusMessage)));
      //qDebug() << "KbvDBusMonitor service org.gtk.vfs.UDisks2VolumeMonitor interface org.gtk.Private.RemoteVolumeMonitor"; //###########
    }

  //Establish interface org.gtk.Private.RemoteVolumeMonitor for service org.gtkvfs.MTPVolumeMonitor
  //connect to signals "List, MountAdded, MountedRemoved" for volumes
  if(dbusDaemon->isServiceRegistered("org.gtk.vfs.MTPVolumeMonitor").value())
    {
      mountedVolumes = new QDBusInterface("org.gtk.vfs.MTPVolumeMonitor", "/org/gtk/Private/RemoteVolumeMonitor",
                                          "org.gtk.Private.RemoteVolumeMonitor", kbvSessionBus, this);
      mountedVolumes->callWithCallback("List", args, this, SLOT(privateRemVolMonList(QDBusMessage)), SLOT(mountError(QDBusError)));
      kbvSessionBus.connect("org.gtk.vfs.MTPVolumeMonitor", "/org/gtk/Private/RemoteVolumeMonitor", "org.gtk.Private.RemoteVolumeMonitor",
                            "MountAdded", this, SLOT(privateRemVolMonMounted(QDBusMessage)));
      kbvSessionBus.connect("org.gtk.vfs.MTPVolumeMonitor", "/org/gtk/Private/RemoteVolumeMonitor", "org.gtk.Private.RemoteVolumeMonitor",
                            "MountRemoved", this, SLOT(privateRemVolMonUnmount(QDBusMessage)));
      //qDebug() << "KbvDBusMonitor service org.gtk.vfs.MTPVolumeMonitor interface org.gtk.Private.RemoteVolumeMonitor"; //###########
    }


/*  
  //register kbvDbusService, future use 
  if(kbvSessionBus.registerObject(QString(kbvDBusPath), this))
    {
      if(kbvSessionBus.registerService(QString(kbvDBusService)))
        {
          
        }
    }
*/
}
KbvDBusMonitor::~KbvDBusMonitor()
{
  //qDebug() << "KbvDBusMonitor::~KbvDBusMonitor"; //###########
  delete  mountedDevices;
  delete  mountedVolumes;
}
/************************************************************************//*!
 * SLOT: service org.gtk.vfs.Daemon
 * interface org.gtk.vfs.MountTracker
 * signal    Lists
 * signature a(sossssssbay(aya(say))ay)
 * structure kbvMountVfs
 */
void   KbvDBusMonitor::vfsMountList(QDBusMessage msg)
{
  QStringList     mountList;
  kbvMountVfs     mount;
  QVariant        var;
  QString         path;
  
  //qDebug() << "KbvDBusMonitor::vfsMountList" <<msg.type() <<msg.signature(); //###########
  var = msg.arguments().at(0);
  const QDBusArgument &dbusarg = var.value<QDBusArgument>();
  dbusarg.beginArray();
  while(!dbusarg.atEnd())     //array of structures kbvMountVfs
    {
      dbusarg.beginStructure();
      dbusarg >> mount.dbusId >> mount.objectPath >> mount.name >> mount.stableName
              >> mount.xContentType >> mount.iconName >> mount.nameEncoding
              >> mount.str >> mount.userVisible >> mount.mountPoint;
      dbusarg.endStructure();
      
      //qDebug() << "KbvDBusMonitor::vfsMountList signature sossssssbay(aya(say))ay"; //###########
      //qDebug() << "KbvDBusMonitor::vfsMountList found:" <<mount.dbusId <<mount.name <<mount.stableName <<mount.userVisible <<mount.mountPoint; //###########
      
      if(mount.userVisible)
        {
          path = mount.mountPoint;
          if(path.startsWith(QString("/")))
            {
              mountList <<mount.name <<path <<mount.iconName;
              emit mountedDevice(mountList);
              //qDebug() << "KbvDBusMonitor::vfsMountList" <<mountList; //###########
            }
        }
    }
  dbusarg.endArray();
}
/************************************************************************//*!
 * SLOT: service org.gtk.vfs.Daemon
 * interface org.gtk.vfs.MountTracker
 * signal    Mounted
 * signature sossssssbay(aya(say))ay
 * structure kbvMountVfs
 */
void   KbvDBusMonitor::vfsMounted(QDBusMessage msg)
{
  QStringList     mountList;
  kbvMountVfs     mount;
  QVariant        var;
  QString         path;

  //qDebug() << "KbvDBusMonitor::vfsMounted" <<msg.type() <<msg.signature(); //###########
  var = msg.arguments().at(0);
  const QDBusArgument &dbusarg = var.value<QDBusArgument>();
  
  dbusarg.beginStructure();
  dbusarg >> mount.dbusId >> mount.objectPath >> mount.name
          >> mount.stableName >> mount.xContentType >> mount.iconName
          >> mount.nameEncoding >> mount.str >> mount.userVisible >> mount.mountPoint;
  dbusarg.endStructure();
  
  if(mount.userVisible)
    {
      if(mount.nameEncoding.isEmpty())
        {
          path = QString::fromUtf8(mount.mountPoint);
        }
      else
        {
          path = QString::fromLatin1(mount.mountPoint);
        }
      if(path.startsWith(QString("/")))
        {
          mountList <<mount.name <<path <<mount.iconName;
          emit mountedDevice(mountList);
          //qDebug() << "KbvDBusMonitor::vfsMounted" <<mountList; //###########
        }
    }
}
/************************************************************************//*!
 * SLOT: service org.gtk.vfs.Daemon
 * interface org.gtk.vfs.MountTracker
 * signal    Unmounted
 * signature sossssssbay(aya(say))ay
 * structure kbvMountVfs
 */
void   KbvDBusMonitor::vfsUnmounted(QDBusMessage msg)
{
  QStringList     mountList;
  kbvMountVfs     mount;
  QVariant        var;
  QString         path;

  //qDebug() << "KbvDBusMonitor::vfsUnmounted" <<msg.type() <<msg.signature(); //###########
  var = msg.arguments().at(0);
  const QDBusArgument &dbusarg = var.value<QDBusArgument>();

  dbusarg.beginStructure();
  dbusarg >> mount.dbusId >> mount.objectPath >> mount.name
          >> mount.stableName >> mount.xContentType >> mount.iconName
          >> mount.nameEncoding >> mount.str >> mount.userVisible >> mount.mountPoint;
  dbusarg.endStructure();

  path = mount.mountPoint;
  if(path.startsWith(QString("/")))
    {
      mountList <<mount.name <<path;
      emit unmountedDevice(mountList);
      //qDebug() << "KbvDBusMonitor::vfsUnmounted" <<mountList; //###########
    }
}

/************************************************************************//*!
 * SLOT: interface org.gtk.Private.RemoteVolumeMonitor
 * signal    Lists
 */
void   KbvDBusMonitor::privateRemVolMonList(QDBusMessage msg)
{
  QStringList       mountList;
  kbvRVMMountStruct mount;
  QVariant          var;
  QString           path;
  
  //qDebug() << "KbvDBusMonitor::privateRemVolMonList" <<msg.type() <<msg.signature(); //###########
  //Ignore the 'drives' and 'volumes', only consider 'mounts'
  var = msg.arguments().at(2);
  
  const QDBusArgument &dbusarg = var.value<QDBusArgument>();
  
  dbusarg.beginArray();
  while(!dbusarg.atEnd())     //array of structures kbvMountVolume
    {
      mountList.clear();
      dbusarg >> mount;

      //qDebug() << "KbvDBusMonitor::privateRemVolMonList struct:" <<mount.id <<mount.name <<mount.volumeId <<mount.mountPoint <<mount.iconNames; //###########

      path = mount.mountPoint;
      if(path.startsWith(QString("file://")))
        {
          path.remove(QString("file://"));
        }
      path.replace(QString("%20"), QString(" "), Qt::CaseInsensitive);
      mountList <<mount.name <<path <<mount.iconNames;
      emit mountedDevice(mountList);
      //qDebug() << "KbvDBusMonitor::privateRemVolMonList" <<mountList; //###########
    }
  dbusarg.endArray();
}
/************************************************************************//*!
 * SLOT: interface org.gtk.Private.RemoteVolumeMonitor
 * signal    MountAdded
*/
void   KbvDBusMonitor::privateRemVolMonMounted(QDBusMessage msg)
{
  QStringList       mountList;
  kbvRVMMountStruct mount;
  QVariant          var;
  QString           path;

  //qDebug() << "KbvDBusMonitor::privateRemVolMonMounted" <<msg.type() <<msg.signature() <<msg.interface(); //###########
  //Ignore the two string parameters before the structure
  var = msg.arguments().at(2);
  const QDBusArgument &dbusarg = var.value<QDBusArgument>();
  dbusarg >> mount;

  path = mount.mountPoint;
  if(path.startsWith(QString("file://")))
    {
      path.remove(QString("file://"));
    }
  path.replace(QString("%20"), QString(" "), Qt::CaseInsensitive);
  mountList <<mount.name <<path <<mount.iconNames;
  emit mountedDevice(mountList);
  //qDebug() << "KbvDBusMonitor::privateRemVolMonMount" <<mountList; //###########
}
/************************************************************************//*!
 * SLOT: interface org.gtk.Private.RemoteVolumeMonitor
 * signal    MountRemoved
 */
void   KbvDBusMonitor::privateRemVolMonUnmount(QDBusMessage msg)
{
  QStringList       mountList;
  kbvRVMMountStruct mount;
  QVariant          var;
  QString           path;

  //qDebug() << "KbvDBusMonitor::privateRemVolMonUnmount" <<msg.type() <<msg.signature(); //###########
  //Ignore the two string parameters before the structure
  var = msg.arguments().at(2);
  const QDBusArgument &dbusarg = var.value<QDBusArgument>();
  dbusarg >> mount;

  path = mount.mountPoint;
  if(path.startsWith(QString("file://")))
    {
      path.remove(QString("file://"));
    }
  path.replace(QString("%20"), QString(" "), Qt::CaseInsensitive);
  mountList <<mount.name <<path;
  emit unmountedDevice(mountList);
  //qDebug() << "KbvDBusMonitor::privateRemVolMonUnmount" <<mountList; //###########
}

/************************************************************************//*!
 * SLOT: Error reply of dbus connections"
 */
void   KbvDBusMonitor::mountError(QDBusError error)
{
  qDebug() << "KbvDBusMonitor::mountError" << error.name() <<error.type() <<error.message(); //###########
}

/************************************************************************//*!
 * Operators << and >> for
 * interface: org.gtk.Private.RemoteVolumeMonitor
 * signature: ssssssbsassa{sv}, structure 'kbvRVMMountStruct'
 */
const QDBusArgument  &operator>>(const QDBusArgument &arg, kbvRVMMountStruct &smv)
{
  arg.beginStructure();
  arg >> smv.id >> smv.name >> smv.iconNames >> smv.symbolicIcon
      >> smv.uuid >> smv.mountPoint >> smv.removable >> smv.volumeId;
  arg.endStructure();
  return arg;
  }
QDBusArgument  &operator<<(QDBusArgument &arg, const kbvRVMMountStruct &smv)
{
  arg.beginStructure();
  arg << smv.id << smv.name << smv.iconNames << smv.symbolicIcon
      << smv.uuid << smv.mountPoint << smv.removable << smv.volumeId;
  arg.endStructure();
  return arg;
}
/************************************************************************//*!
 * Operators << and >> for
 * interface: org.gtk.vfs.MountTracker
 * signature: sosssssbay(aya(say))ay -> structure kbvMountVfs
 */
const QDBusArgument   &operator>>(const QDBusArgument &arg, kbvMountVfs &smd)
{
  arg.beginStructure();
  arg >> smd.dbusId >> smd.objectPath >> smd.name >> smd.stableName >> smd.xContentType
      >> smd.iconName >> smd.nameEncoding >> smd.str >> smd.userVisible >> smd.mountPoint;
  arg.endStructure();
  return arg;
}
QDBusArgument   &operator<<(QDBusArgument &arg, const kbvMountVfs &smd)
{
  arg.beginStructure();
  arg << smd.dbusId << smd.objectPath << smd.name << smd.stableName << smd.xContentType
      << smd.iconName << smd.nameEncoding << smd.str << smd.userVisible << smd.mountPoint;
  arg.endStructure();
  return arg;
}
/****************************************************************************/
