#ifndef TASKQUEUE_HPP
#define TASKQUEUE_HPP

#include <QThread>
#include <QList>
#include <QVector>
#include <QMutex>

#include <taskinterface.hpp>

class taskQueue : public QThread
{
  Q_OBJECT
public:
  typedef	enum status_enum_t{state_idle=0, state_run, state_stop} status_enum;

  explicit taskQueue(QObject *parent = 0);
  ~taskQueue();

  // Append a task to the current list
  void addTask(taskInterface *task);
  // Stops thread after current task, deleting all tasks after that.
  void clearTasks(void);
  // Stops thread execution after current task
  void stopTasks(void);
  // Get the current state
  status_enum getState();

  // Runs inside the task thread
  void run(void);

signals:
public slots:
private:
  // Start thread if the thread is not running
  void startIfNotStarted();
  // Set mode in a thread safe way
  void setState(status_enum state);

  // Mutex to protect list and mode.
  QMutex mutex;

  // QList holding tasks
  QVector<taskInterface*> tasks;
  //QVector<taskInterface*>::iterator current;
  int current_task;

  /// modes:
  // state_idle	: The task queue is not running.
  // state_run	: The task queue is currently running.
  // state_stop	: The execution of tasks stops after this task is finished.
  status_enum current_state;
};

#endif
