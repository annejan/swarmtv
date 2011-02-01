#include <QList>
#include <QThread>
#include <QObject>

#include "taskqueue.hpp"

//taskQueue::taskQueue(QObject *parent)
taskQueue::taskQueue(QObject *)
{
  // Start Clean
  tasks.clear();
  current_task=0;
}

taskQueue::~taskQueue()
{
  // set stop state
  stopTasks();
  // wait for the thread to be done
  this->wait(1000);
}

// Set mode in a thread safe way
void taskQueue::setState(status_enum state)
{
  mutex.lock();
  current_state = state;
  mutex.unlock();
}

// Append a task to the current list
void taskQueue::addTask(taskInterface *task)
{
  mutex.lock();
  if(tasks.empty() == true) {
    // When the first task is inserted initialize the iterator
    tasks.append(task);
    //current = tasks.begin();
    current_task = 0;
  } else {
    tasks.append(task);
  }
  mutex.unlock();

  // Start processing tasks
  startIfNotStarted();
}

// Stops thread after current task, deleting all tasks after that.
void taskQueue::clearTasks(void)
{
  // Stop the tasks
  stopTasks();
  // Wait for the thread to stop
  wait();
  // Free all tasks in the Task queue
  qDeleteAll(tasks);
  tasks.clear();
}

// Stops thread execution after current task
void taskQueue::stopTasks(void)
{
  setState(state_stop);
}

// Get the current state
taskQueue::status_enum taskQueue::getState()
{
  mutex.lock();
  return this->current_state;
  mutex.unlock();
}

// Runs inside the task thread
void taskQueue::run(void)
{
  taskInterface* currentTask=NULL;

  while(current_task < tasks.size() && current_state == state_run){
    currentTask = tasks[current_task];
    currentTask->start();
    current_task++;
  }

  // State to idle
  current_state = state_idle;
}

void taskQueue::startIfNotStarted()
{
  // Check if the thread is running
  mutex.lock();
  if(this->isRunning() == false) {
    current_state = state_run;
    this->start();
  }
  mutex.unlock();
}

