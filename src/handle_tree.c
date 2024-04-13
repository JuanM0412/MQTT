#include <stdio.h> 
#include "../include/handle_tree.h"

Tree* get_tree() {
    static Tree singleton;
    static int initialized = 0;

    if (!initialized) {
        singleton.tree = createTreeNode("/");
        pthread_mutex_init(&singleton.mutex, NULL);
        initialized = 1;
    }

    return &singleton;
}

void insert_publish(const char *topic, const char *message) {
    Tree *singleton_tree = get_tree();

    pthread_mutex_lock(&singleton_tree->mutex);

    publish(singleton_tree->tree, topic, message);
    printf("Mensajeeee: %s\n", message);
    printTree(singleton_tree->tree, 0);

    pthread_mutex_unlock(&singleton_tree->mutex);
}

void insert_subscribe(const char *topic, int connfd) {
    Tree *singleton_tree = get_tree();

    pthread_mutex_lock(&singleton_tree->mutex);

    subscribe(singleton_tree->tree, topic, connfd, 0);
    printTree(singleton_tree->tree, 0);

    pthread_mutex_unlock(&singleton_tree->mutex);
}