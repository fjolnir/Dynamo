// Dynamo view controller

// Sets up an GLKView and initializes dynamo using the supplied boot script
#import <GLKit/GLKit.h>
#import <dynamo/lua.h>

@interface DViewController : GLKViewController {
    EAGLContext *_context;
    LuaContext_t *_luaCtx;
    NSMutableArray *_activeTouches;
    NSString *_bootScriptPath;
}
@property(readonly) NSString *bootScriptPath;

- (id)initWithBootScriptPath:(NSString *)aPath;
@end
