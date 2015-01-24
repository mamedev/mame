/*
 * debug_module.h
 *
 */

#ifndef DEBUG_MODULE_H_
#define DEBUG_MODULE_H_

class debug_module
{
public:

    virtual ~debug_module() { }

    virtual void init_debugger(running_machine &machine) = 0;
    virtual void wait_for_debugger(device_t &device, bool firststop) = 0;
    virtual void debugger_update() = 0;
    virtual void debugger_exit() = 0;
};



#endif /* DEBUG_MODULE_H_ */
