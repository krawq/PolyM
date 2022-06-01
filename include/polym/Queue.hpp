#pragma once
#include "circular_q.hpp"
#include "Msg.hpp"
#include <memory>
#include <condition_variable>
#include <unordered_map>

namespace PolyM {

    struct Request
    {
        Request() {};

        std::unique_ptr<Msg> response;
        std::condition_variable condVar;
    };
/**
 * Queue is a thread-safe message queue.
 * It supports one-way messaging and request-response pattern.
 */
class Queue
{
public:
    Queue();

    ~Queue();

    /**
     * Put Msg to the end of the queue.
     *
     * @param msg Msg to put to the queue.
     */
    void put(Msg&& msg);

    /**
     * Get message from the head of the queue.
     * Blocks until at least one message is available in the queue, or until timeout happens.
     * If get() returns due to timeout, returns a nullptr.
     *
     * @param timeoutMillis How many ms to wait for message until timeout happens.
     *                      0 = wait indefinitely.
     */
    std::unique_ptr<Msg> get(int timeoutMillis = 0);

    /**
    * Get message from the head of the queue.
    * Returns an empty pointer if no message is available.
    */
    std::unique_ptr<Msg> tryGet();

    /**
     * Make a request.
     * Call will block until response is given with respondTo().
     * If request() returns due to timeout, returns a nullptr.
     * 
     * @param msg Request message. Is put to the queue so it can be retrieved from it with get().
     * @param timeoutMillis How many ms to wait for response until timeout happens.
     *                      0 = wait indefinitely.
     */
    std::unique_ptr<Msg> request(Msg&& msg, int timeoutMillis = 0);

    /**
     * Respond to a request previously made with request().
     * If the requestID has been found, return true.
     * 
     * @param reqUid Msg UID of the request message.
     * @param responseMsg Response message. The requester will receive it as the return value of
     *                    request().
     */
    bool respondTo(MsgUID reqUid, Msg&& responseMsg);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};


class CircularQueue
{
public:
    explicit CircularQueue(size_t max_items);

    /**
     * Put Msg to the end of the queue.
     *
     * @param msg Msg to put to the queue.
     */
    void put(Msg&& msg);

    /**
     * Get message from the head of the queue.
     * Blocks until at least one message is available in the queue, or until timeout happens.
     * If get() returns due to timeout, returns a nullptr.
     *
     * @param timeoutMillis How many ms to wait for message until timeout happens.
     *                      0 = wait indefinitely.
     */
    std::unique_ptr<Msg> get(int timeoutMillis = 0);

    /**
    * Get message from the head of the queue.
    * Returns an empty pointer if no message is available.
    */
    std::unique_ptr<Msg> tryGet();

    /**
     * Make a request.
     * Call will block until response is given with respondTo().
     * If request() returns due to timeout, returns a nullptr.
     *
     * @param msg Request message. Is put to the queue so it can be retrieved from it with get().
     * @param timeoutMillis How many ms to wait for response until timeout happens.
     *                      0 = wait indefinitely.
     */
    std::unique_ptr<Msg> request(Msg&& msg, int timeoutMillis = 0);

    /**
     * Respond to a request previously made with request().
     * If the requestID has been found, return true.
     *
     * @param reqUid Msg UID of the request message.
     * @param responseMsg Response message. The requester will receive it as the return value of
     *                    request().
     */
    bool respondTo(MsgUID reqUid, Msg&& responseMsg);

private:
    // Queue for the Msgs
    circular_q<std::unique_ptr<Msg>> queue_;

    // Mutex to protect access to the queue
    std::mutex queueMutex_;

    // Condition variable to wait for when getting Msgs from the queue
    std::condition_variable push_cv_;

    // Condition variable to wait for when puting Msgs in the queue
    std::condition_variable pop_cv_;

    // Map to keep track of which request IDs are associated with which request Msgs
    std::unordered_map<MsgUID, Request*> responseMap_;

    // Mutex to protect access to response map
    std::mutex responseMapMutex_;
};

}
