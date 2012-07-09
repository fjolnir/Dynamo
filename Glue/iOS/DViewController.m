#import "DViewController.h"
#import <dynamo/input.h>
#import <dynamo/luacontext.h>

const NSString *kDynamoMessageNotification = @"DynamoMessageNotification";

@interface DViewController (Private)
- (void)setupGL;
- (void)tearDownGL;
@end

@implementation DViewController
@synthesize bootScriptPath=_bootScriptPath;

- (id)initWithBootScriptPath:(NSString *)aPath;
{
    if(!(self = [super init]))
        return nil;

    _bootScriptPath = [aPath copy];

    return self;
}

- (void)loadView
{
    GLKView *view = [[[GLKView alloc] initWithFrame:[[UIScreen mainScreen] applicationFrame]] autorelease];
    self.view = view;

    view.drawableColorFormat   = GLKViewDrawableColorFormatRGB565;
    view.drawableDepthFormat   = GLKViewDrawableDepthFormatNone;
    view.drawableStencilFormat = GLKViewDrawableStencilFormatNone;
    view.drawableMultisample   = GLKViewDrawableMultisampleNone;
}

- (void)dealloc
{
    luaCtx_getglobal(GlobalLuaContext, "dynamo");
    luaCtx_getfield(GlobalLuaContext, -1, "cleanup");
    luaCtx_pcall(GlobalLuaContext, 0, 0, 0);
    luaCtx_pop(GlobalLuaContext, 1);

    luaCtx_teardown();

    [_context release], _context = nil;
    [_activeTouches release]; _activeTouches = nil;
    [_bootScriptPath release], _bootScriptPath = nil;

    [super dealloc];
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    self.view.multipleTouchEnabled = YES;

    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    dynamo_assert(_context != nil, "Couldn't create OpenGL ES 2 context");

    _activeTouches = [[NSMutableArray alloc] init];

    self.preferredFramesPerSecond = 60;
    GLKView *view = (GLKView *)self.view;
    view.context = _context;

    [self setupGL];
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    [_activeTouches release], _activeTouches = nil;

    [self tearDownGL];

    if([EAGLContext currentContext] == _context)
        [EAGLContext setCurrentContext:nil];
    [_context release], _context = nil;
}

- (void)_postTouchEventWithFinger:(int)aFinger isDown:(BOOL)aIsDown location:(CGPoint)aLoc
{
    aLoc.y = self.view.bounds.size.height - aLoc.y;
    float scaleFactor = self.view.contentScaleFactor;

    luaCtx_getglobal(GlobalLuaContext, "dynamo");
    luaCtx_getfield(GlobalLuaContext, -1, "input");
    luaCtx_getfield(GlobalLuaContext, -1, "manager");
    luaCtx_getfield(GlobalLuaContext, -1, "postTouchEvent");

    luaCtx_pushvalue(GlobalLuaContext, -2);
    luaCtx_pushinteger(GlobalLuaContext, aFinger);
    luaCtx_pushboolean(GlobalLuaContext, aIsDown);
    luaCtx_pushnumber(GlobalLuaContext, aLoc.x*scaleFactor);
    luaCtx_pushnumber(GlobalLuaContext, aLoc.y*scaleFactor);
    luaCtx_pcall(GlobalLuaContext, 5, 0, 0);

    luaCtx_pop(GlobalLuaContext, 3);
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:_context];

    luaCtx_init();
    NSString *dynamoScriptsPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"DynamoScripts"];
    NSString *localScriptsPath  = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"Scripts"];
    luaCtx_addSearchPath(GlobalLuaContext, [dynamoScriptsPath fileSystemRepresentation]);
    luaCtx_addSearchPath(GlobalLuaContext, [localScriptsPath fileSystemRepresentation]);

    [self prepareLuaContext];

    dynamo_assert(luaCtx_executeFile(GlobalLuaContext, [_bootScriptPath fileSystemRepresentation]), "Lua error");
}

- (void)prepareLuaContext
{
    // Subclasses can hook in  here to initialize the lua context before the boot script is run
}

- (void)tearDownGL
{
    [EAGLContext setCurrentContext:_context];
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    luaCtx_getglobal(GlobalLuaContext, "dynamo");
    luaCtx_getfield(GlobalLuaContext, -1, "cycle");
    luaCtx_pcall(GlobalLuaContext, 0, 1, 0);
    if(luaCtx_istable(GlobalLuaContext, -1)) {
        luaCtx_pushnil(GlobalLuaContext);
        NSString *key;
        id value;
        while(luaCtx_next(GlobalLuaContext, -2) != 0) {
            key = [NSString stringWithUTF8String:luaCtx_tostring(GlobalLuaContext, -2)];
            if(luaCtx_isnumber(GlobalLuaContext, -1))
                value = [NSNumber numberWithFloat:luaCtx_tonumber(GlobalLuaContext, -1)];
            else if(luaCtx_isstring(GlobalLuaContext, -1))
                value = [NSString stringWithUTF8String:luaCtx_tostring(GlobalLuaContext, -1)];
            else {
                dynamo_log("Unhandled message type for key %s", [key UTF8String]);
                continue;
            }
            NSDictionary *notDic = [NSDictionary dictionaryWithObject:value forKey:key];
            [[NSNotificationCenter defaultCenter] postNotificationName:(NSString*)kDynamoMessageNotification
                                                                object:self
                                                              userInfo:notDic];
            luaCtx_pop(GlobalLuaContext, 1);
        }
    }
    luaCtx_pop(GlobalLuaContext, 2);
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    for(UITouch *touch in touches) {
        [_activeTouches addObject:touch];
        [self _postTouchEventWithFinger:[_activeTouches indexOfObject:touch]
                                 isDown:YES
                               location:[touch locationInView:self.view]];
    }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    for(UITouch *touch in touches) {
        [self _postTouchEventWithFinger:[_activeTouches indexOfObject:touch]
                                 isDown:YES
                               location:[touch locationInView:self.view]];
    }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    for(UITouch *touch in touches) {
        [self _postTouchEventWithFinger:[_activeTouches indexOfObject:touch]
                                 isDown:NO
                               location:[touch locationInView:self.view]];
        // If this is the last event, reset the active event so that the next touch starts at index 0
        if([event touchesForView:self.view].count == 1)
            [_activeTouches removeAllObjects];
    }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    for(UITouch *touch in touches) {
        [_activeTouches removeObject:touch];
    }
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return interfaceOrientation == UIInterfaceOrientationPortrait;
}


@end
