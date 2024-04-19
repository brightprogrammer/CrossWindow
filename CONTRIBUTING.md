
# Contribution Guide

I welcome contribution from anyone as long as the follow a set of strict guidelines that need to be followed.

## Code Uniformity / Code Formatting
Make sure to use the `.clang-format` present in repository root to format the code. That way you don't need to worry about tabs/spaces and other formatting debates and we'll make sure the code looks uniform everywhere.
  
  If your editor does not support it then you can install the command line too and perform `clang-format -i <code.c/h> --style=file:.clang-format`  

Just fire `clang-format `on your code, and forget about proper code-formatting at all!

## Naming Conventions
- Follow `PascalCase` for naming of data types and `snake_case` for naming of methods.
- If you create a new type with name `MyNewType`, the functions must start (be prefixed) with something similar :
    - `my_new_type_xyzt`
    - `my_type_xyzt`
Where xyzt is the name of actual method. 
- Don't leave structs to be used as `struct MyNewType`, `typedef` them at the moment they're defined/declared. This will make sure we can use `MyNewType` directly instead of using it as `struct MyNewType`. You can obviously split the `typedef` and struct definition. Basically you just need to make that typedef available to user-code anyhow.

Here is an example following all the above principles
```c
/* typedef separately */
struct Vec2 {
    Float32 x;
    Float32 y;
};
typedef struct Vec2 Vec2;

/* typedef in place */
typedef struct Vec3 {
    Float32 x;
    Float32 y;
    Float32 z;
} Vec3;

/* since type is Vec3, prefix will be "vec3" and actual method name is "add",
 * so that will be suffixed to "vec3_"
 */
Vec3 vec3_add(Vec3 a, Vec3 b) {
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}
```

## Defining/Declaring/Initializing Variables

- Always make sure to initialize variable to some invalid value before use it. If the variable is storing return value of a method, and it's possible to do that directly then store the result directly instead of first declaring the variable and then using it. 
- You can make an exception to above rule if you it's guaranteed that variables will be properly initialized before use.

Following is example of above rule not being followed
```c
/* x is not guaranteed to be initialized */
Int32 check_and_compute_value(...) {
    Int32 x;
    if(check_something_and_get_value(...)) {
        x = compute_value();
    }
    return x;
}

Int32 get_query_result(QueryResult** res) {
        /* some code here */
}

/* res must be initialized as Null here! */
QueryResult* res;
get_query_result(&res);
```

Following is corrected example : 
```c
/* x is now guaranteed to be initialized */
Int32 check_and_compute_value(...) {
    Int32 x = -1; /* treating -1 as invalid value here */
    if(check_something_and_get_value(...)) {
        x = compute_value();
    }
    return x;
}

void query_result_get(QueryResult** res) {
        /* some code here */
}

/* res is Null and now we can check for errors using it */
QueryResult* res = Null;
query_result_get(&res);
if(!res) {
    /* didn't get result, handle error here */
} else {
    /* got query result */
    query_process()
}
```

## Return Value Conventions

All methods **MUST** return something for error checking, given the function is viable for failure. If the function cannot fail at all in any scenario, then it must return `void`.

In general, if we're dealing with objects, then the object itself must be returned. This helps in error checking of code. User code can then decide what functions to check errors for, and what not to check.

Destroy methods must always return `void`. Even if they fail, we cannot do anything about it! Destructor methods are exempted from above rules.

Following are some examples following the above convention
```c
typedef struct LogicalDevice {
    VkDevice         device;
    VkPhysicalDevice gpu;
} LogicalDevice;

/* create methods allocate memory for new object */
LogicalDevice* logical_device_create(VkPhysicalDevice gpu);

/* initialize methods actually initialize the object */
LogicalDevice* logical_device_init(LogicalDevice* self, VkPhysicalDevice gpu);

/* deinitialize free all resources allocated in init() method */
LogicalDevice *logical_device_deinit(LogicalDevice* self);

/* destroy will call deinit() and free() on object*/
void logical_device_destroy(LogicalDevice* self);
```

Above code also gives example of 4 must present functions for each object type. This allows user to escape allocation of `LogicalDevice` or any object for that matter on heap. 

## Error Handling

The file `Include/Common.h` provides some macros for generic error handling.
- `RETURN_IF(cond, msg)`
- `RETURN_VALUE_IF(cond, value, msg)`
- `ABORT_IF(cond, msg)`
- `GOTO_HANDLER_IF(cond, label, msg)`
- ... and many others

Below are examples of cases where one might use them : 
```c
/* RETURN_IF is used when method does not return anything (void) */
void logical_device_destroy(LogicalDevice* device) {
    RETURN_IF(!device, ERR_INVALID_ARGUMENTS);
    
    logical_device_deinit(device);
        
    /* always use FREE, NEW, ALLOCATE and REALLOCATE macros for memory management */
    FREE(device);
}

LogicalDevice* logical_device_create(VkPhysicalDevice gpu) {
    RETURN_VALUE_IF(!gpu, Null, ERR_INVALID_ARGUMENTS);
    
    LogicalDevice* dev = NEW(LogicalDevice);
    RETURN_VALUE_IF(!dev, Null, ERR_OUT_OF_MEMORY);

    LogicalDevice* idev = logical_device_init(dev, gpu);
    if(!dev) {
        PRINT_ERR(ERR_OBJECT_INITIALIZATION_FAILED);
        /* call destroy and not FREE! */
        logical_device_destroy(dev);
        return Null;
    }

    return idev;
}

/* alternatively one can use GOTO_HANDLER_IF macro here */
LogicalDevice* logical_device_create(VkPhysicalDevice gpu) {
    RETURN_VALUE_IF(!gpu, Null, ERR_INVALID_ARGUMENTS);
    
    LogicalDevice* dev = NEW(LogicalDevice);
    RETURN_VALUE_IF(!dev, Null, ERR_OUT_OF_MEMORY);

    LogicalDevice* idev = logical_device_init(dev, gpu);
    GOTO_HANDLER_IF(!idev, INIT_FAILED, ERR_OBJECT_INITIALIZATION_FAILED);
        
    return idev;

INIT_FAILED:
    logical_device_destroy(dev);
    return Null;
}
```

## Public/Private Methods/Objects
If user-code does not require to see some method, make sure to move that method to a `.c` file or a file that user will never include in their code using `#include`. If possible, define that method as static to limit the scope of that method to that source file only.

I refer to these static methods as private methods, because they're analogous to `private` in C++.

Make sure to forward declare all private methods and define them at the
very bottom of source file that includes the definition. Separate all private
method declarations by a huge line like the following : 

```c
/***************************** PRIVATE METHODS ***************************/
```

Symmetry of this line is not required as long as it's discernible when skimming through the code,

Also, make sure that if user does not need to see the internals of an object, then keep it as opaque object. One example is when user can mess up the internal data of an object if allowed to change it without some kind of check, eg : Window size methods. Setting window size, we need to check whether it is in bounds or not. For these objects, to change any property, we need `get()` and `set()` methods.

## Code Documentation

This is one of the very important rules of all! Make sure to document all methods! Be it public/private, add doxygen style documentation. If it's required, explain why you took that particular approach to solving problem at hand. This will make the potential barrier of new contributors quite low.

That's all!
