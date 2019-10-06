#ifndef BASE_BLOCKINGQUEUE_H
#define BASE_BLOCKINGQUEUE_H

#include<boost/circular_buffer.hpp>
#include<boost/noncopyable.hpp>
#include "base/Mutex.h"
#include "base/Condition.h"


template<typename T>
class Blockingqueue: boost::noncopyable
{
    public:
        explicit Blockingqueue(int maxsize)
            :mutex_(),
            notEmpty_(mutex_),
            notFull_(mutex_),
            queue_(maxsize),
            unread_(0)
        {
        }

        void put(T& x)
        {
            MutexLockGuard lock(mutex_);
            while(unread_ == queue_.capacity())
            {
                notFull_.wait();
            }
            assert(unread_ < queue_.capacity());
            queue_.push_front(x);
            ++unread_;
            notEmpty_.notify();
        }

        T take()
        {
            MutexLockGuard lock(mutex_);
            while(unread_ == 0)
            {
                notEmpty_.wait();
            }
            assert(unread_ > 0);
            T front = queue_[--unread_];
            notFull_.notify();
            return std::move(front);
        }


        bool empty()const
        {
            MutexLockGuard lock(mutex_);
            return queue_.empty();
        }

        bool full()const
        {
            MutexLockGuard lock(mutex_);
            return queue_.full();
        }

        size_t size()const
        {
            MutexLockGuard lock(mutex_);
            return queue_.size();
        }

        size_t capacity() const
        {
            MutexLockGuard lock(mutex_);
            return queue_.capacity();
        }


    private:
        mutable MutexLock mutex_;
        Condition notEmpty_;
        Condition notFull_;
        boost::circular_buffer<T> queue_;
        size_t unread_;// protected by mutex_
};


template<typename T>
class Blockingqueue<T*>
{
      public:
        explicit Blockingqueue(int maxsize)
            :mutex_(),
            notEmpty_(mutex_),
            notFull_(mutex_),
            queue_(maxsize),
            unread_(0)
        {
        }

        void put(T* x)
        {
            MutexLockGuard lock(mutex_);
            while(unread_ == queue_.capacity())
            {
                notFull_.wait();
            }
            assert(unread_ < queue_.capacity());
            queue_.push_front(x);
            ++unread_;
            notEmpty_.notify();
        }

        T* take()
        {
            MutexLockGuard lock(mutex_);
            while(unread_ == 0)
            {
                notEmpty_.wait();
            }
            assert(unread_ > 0);
            T* front = queue_[--unread_];
            notFull_.notify();
            return front;
        }

    private:
        mutable MutexLock mutex_;
        Condition notEmpty_;
        Condition notFull_;
        boost::circular_buffer<T*> queue_;
        size_t unread_;// protected by mutex_     
};


#endif