#pragma once
#include "common.h"

struct queue_node {
    struct queue_node *next;
    struct queue_node *prev;
};

struct queue {
    struct queue_node *head;
    struct queue_node *tail;
};

static struct queue *queue_alloc() {
    struct queue *q = malloc(sizeof(struct queue));
    q->head = NULL;
    q->tail = NULL;

    return q;
}

static struct queue queue_create() {
    struct queue q;
    q.head = NULL;
    q.tail = NULL;

    return q;
}

static void queue_init_node(struct queue_node *node) {
    node->next = NULL;
    node->prev = NULL;
}

static void queue_push(struct queue *q, struct queue_node *node) {
    if (q->head == NULL) {
        q->head = node;
    } else {
        q->tail->next = node;
        node->prev = q->tail;
    }
    q->tail = node;
}

static struct queue_node *queue_pop(struct queue *q) {
    if (q->head == NULL) {
        return NULL;
    }

    struct queue_node *found = q->head;
    
    if (q->head == q->tail) {
        q->head = NULL;
        q->tail = NULL;

        found->next = NULL;
        found->prev = NULL;
        
        return found;
    }

    q->head = found->next;
    q->head->prev = NULL;

    found->next = NULL;
    found->prev = NULL;

    return found;
}