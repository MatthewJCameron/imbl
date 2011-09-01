#ifndef COMPONENT_H
#define COMPONENT_H

#include<QObject>
#include<QString>

class Component : public QObject {
  Q_OBJECT;

private:
  bool iAmConnected;
  QString desc;

public:

  Component(const QString & name=QString(), QObject * parent=0);
  Component(QObject * parent);

  bool isConnected() const;
  const QString & description() const;
  bool getReady(int delay=0);

protected slots:
  void setDescription(const QString & newDescription);
  void setConnected(bool newConnection);

signals:
  void connectionChanged(bool connection);
  void descriptionChanged(const QString & newDescription);

};

#endif // COMPONENT_H
