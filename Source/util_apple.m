#import <Foundation/Foundation.h>

// Little wrapper to enable use of NSLog from .c files (CFLog is private)
void _dynamo_wrapped_NSLog(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    NSLogv([NSString stringWithUTF8String:fmt], args);
    va_end(args);
}
