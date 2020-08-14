#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{

    std::unique_lock<std::mutex> uniqueLock(_mutex);
    _condition.wait(uniqueLock,[this]{ return !_queue.empty();});

    T message = std::move(_queue.back());
    _queue.pop_back();
    return message;
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> lck(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if(queueMessage.receive() == TrafficLightPhase::green){
            return;
        }
    }
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases,this));

    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{

    auto updateTime = std::chrono::system_clock::now();

    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_real_distribution<> distr(4000.0, 6000.0);
    
    // setup variables
    double cycle = distr(eng); // duration of a single simulation cycle in ms
    // auto cycle = std::rand() % 6000 + 4000;

    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        long timeElapsed =std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - updateTime).count();
        if(timeElapsed>cycle){

            if(_currentPhase == TrafficLightPhase::green){
                _currentPhase = TrafficLightPhase::red;
            }
            else 
            {
                _currentPhase = TrafficLightPhase::green;
            }

            queueMessage.send(std::move(_currentPhase));

            updateTime = std::chrono::system_clock::now();

        }
        // std::cout<<cycle;
    }
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
}

