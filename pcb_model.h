#ifndef PCB_MODEL_H
#define PCB_MODEL_H

#include <stdbool.h>

// Data structures for PCB model
typedef struct {
    double x;
    double y;
} Point;

typedef struct {
    int number;
    char* name;
    char* function;
} Pin;

typedef struct {
    char* name;
    char* package;
    char* manufacturer;
    char* mpn;
    Pin* pins;
    int pin_count;
    int pin_capacity;
} Component;

typedef struct {
    char* ref_designator;  // e.g., "U1"
    char* component_name;  // e.g., "ATmega328P"
    Point position;
    int rotation;
    bool top_side;
} Placement;

typedef struct {
    char* instance;
    char* pin_name;
    int pin_number;  // -1 if not specified
} PinReference;

typedef struct {
    char* name;
    PinReference* connections;
    int connection_count;
    int connection_capacity;
} Net;

typedef struct {
    char* name;
    double width;
    double height;
    int layers;
    Component* components;
    int component_count;
    int component_capacity;
    Placement* placements;
    int placement_count;
    int placement_capacity;
    Net* nets;
    int net_count;
    int net_capacity;
} Board;

// Function declarations
Board* create_board();
Component* create_component(char* name);
Placement* create_placement();
Net* create_net(char* name);

void add_component(Board* board, Component* component);
void add_pin_to_component(Component* component, Pin pin);
void add_placement(Board* board, Placement* placement);
void add_net(Board* board, Net* net);
void add_connection_to_net(Net* net, PinReference pin_ref);

void free_board(Board* board);

#endif // PCB_MODEL_H