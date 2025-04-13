#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcb_model.h"

static void check_allocation(void* ptr, const char* error_message) {
    if (!ptr) {
        fprintf(stderr, "Memory allocation failed: %s\n", error_message);
        exit(EXIT_FAILURE);
    }
}

Board* create_board() {
    Board* board = (Board*)malloc(sizeof(Board));
    check_allocation(board, "creating board");
    
    board->name = NULL;
    board->width = 0;
    board->height = 0;
    board->layers = 2;
    
    board->component_capacity = 10;
    board->components = (Component*)malloc(board->component_capacity * sizeof(Component));
    check_allocation(board->components, "initializing components array");
    board->component_count = 0;
    
    board->placement_capacity = 10;
    board->placements = (Placement*)malloc(board->placement_capacity * sizeof(Placement));
    check_allocation(board->placements, "initializing placements array");
    board->placement_count = 0;
    
    board->net_capacity = 10;
    board->nets = (Net*)malloc(board->net_capacity * sizeof(Net));
    check_allocation(board->nets, "initializing nets array");
    board->net_count = 0;
    
    return board;
}

Component* create_component(char* name) {
    Component* component = (Component*)malloc(sizeof(Component));
    check_allocation(component, "creating component");
    
    component->name = name;
    component->package = NULL;
    component->manufacturer = NULL;
    component->mpn = NULL;
    component->pins = NULL;
    component->pin_count = 0;
    component->pin_capacity = 0;
    
    return component;
}

Placement* create_placement() {
    Placement* placement = (Placement*)malloc(sizeof(Placement));
    check_allocation(placement, "creating placement");
    
    placement->ref_designator = NULL;
    placement->component_name = NULL;
    placement->position.x = 0;
    placement->position.y = 0;
    placement->rotation = 0;
    placement->top_side = true;
    
    return placement;
}

Net* create_net(char* name) {
    Net* net = (Net*)malloc(sizeof(Net));
    check_allocation(net, "creating net");
    
    net->name = name;
    net->connection_capacity = 10;
    net->connections = (PinReference*)malloc(net->connection_capacity * sizeof(PinReference));
    check_allocation(net->connections, "initializing connections array");
    net->connection_count = 0;
    
    return net;
}

void add_component(Board* board, Component* component) {
    if (board->component_count >= board->component_capacity) {
        board->component_capacity *= 2;
        board->components = (Component*)realloc(board->components, 
                                               board->component_capacity * sizeof(Component));
        check_allocation(board->components, "expanding components array");
    }
    
    board->components[board->component_count++] = *component;
}

void add_pin_to_component(Component* component, Pin pin) {
    if (component->pin_count >= component->pin_capacity) {
        component->pin_capacity = component->pin_capacity == 0 ? 4 : component->pin_capacity * 2;
        component->pins = (Pin*)realloc(component->pins, 
                                       component->pin_capacity * sizeof(Pin));
        check_allocation(component->pins, "expanding pins array");
    }
    
    component->pins[component->pin_count++] = pin;
}

void add_placement(Board* board, Placement* placement) {
    if (board->placement_count >= board->placement_capacity) {
        board->placement_capacity *= 2;
        board->placements = (Placement*)realloc(board->placements, 
                                               board->placement_capacity * sizeof(Placement));
        check_allocation(board->placements, "expanding placements array");
    }
    
    board->placements[board->placement_count++] = *placement;
}

void add_net(Board* board, Net* net) {
    if (board->net_count >= board->net_capacity) {
        board->net_capacity *= 2;
        board->nets = (Net*)realloc(board->nets, 
                                   board->net_capacity * sizeof(Net));
        check_allocation(board->nets, "expanding nets array");
    }
    
    board->nets[board->net_count++] = *net;
}

void add_connection_to_net(Net* net, PinReference pin_ref) {
    if (net->connection_count >= net->connection_capacity) {
        net->connection_capacity *= 2;
        net->connections = (PinReference*)realloc(net->connections, 
                                                 net->connection_capacity * sizeof(PinReference));
        check_allocation(net->connections, "expanding connections array");
    }
    
    net->connections[net->connection_count++] = pin_ref;
}

void free_board(Board* board) {
    if (!board) return;
    
    if (board->name) free(board->name);
    
    for (int i = 0; i < board->component_count; i++) {
        Component* comp = &board->components[i];
        if (comp->name) free(comp->name);
        if (comp->package) free(comp->package);
        if (comp->manufacturer) free(comp->manufacturer);
        if (comp->mpn) free(comp->mpn);
        
        for (int j = 0; j < comp->pin_count; j++) {
            Pin* pin = &comp->pins[j];
            if (pin->name) free(pin->name);
            if (pin->function) free(pin->function);
        }
        
        if (comp->pins) free(comp->pins);
    }
    
    if (board->components) free(board->components);
    
    for (int i = 0; i < board->placement_count; i++) {
        Placement* place = &board->placements[i];
        if (place->ref_designator) free(place->ref_designator);
        if (place->component_name) free(place->component_name);
    }
    
    if (board->placements) free(board->placements);
    
    for (int i = 0; i < board->net_count; i++) {
        Net* net = &board->nets[i];
        if (net->name) free(net->name);
        
        for (int j = 0; j < net->connection_count; j++) {
            PinReference* ref = &net->connections[j];
            if (ref->instance) free(ref->instance);
            if (ref->pin_name) free(ref->pin_name);
        }
        
        if (net->connections) free(net->connections);
    }
    
    if (board->nets) free(board->nets);
    free(board);
}