#import "DViewController.h"
#import <Dynamo/input.h>

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

	_luaCtx = obj_retain(luaCtx_createContext());
	_bootScriptPath = [aPath copy];
	
	return self;
}

- (void)loadView
{
	GLKView *view =[[GLKView alloc] initWithFrame:[[UIScreen mainScreen] applicationFrame]];
	self.view = view;
	[view release];
	view.drawableColorFormat = GLKViewDrawableColorFormatRGB565;
	view.drawableDepthFormat = GLKViewDrawableDepthFormatNone;
	view.drawableStencilFormat = GLKViewDrawableStencilFormatNone;
	view.drawableMultisample = GLKViewDrawableMultisampleNone;
}

- (void)dealloc
{
	lua_State *ls = _luaCtx->luaState;
	lua_getglobal(ls, "dynamo");
	lua_getfield(ls, -1, "cleanup");
	luaCtx_pcall(_luaCtx, 0, 0, 0);
	lua_pop(_luaCtx->luaState, 1);
	
	obj_release(_luaCtx), _luaCtx = nil; 
	
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
	lua_State *ls = _luaCtx->luaState;
	float scaleFactor = self.view.contentScaleFactor;
	
	lua_getglobal(ls, "dynamo");
	lua_getfield(ls, -1, "input");
	lua_getfield(ls, -1, "manager");
	lua_getfield(ls, -1, "postTouchEvent");
	
	lua_pushvalue(ls, -2);
	lua_pushinteger(ls, aFinger);
	lua_pushboolean(ls, !aIsDown);
	lua_pushnumber(ls, aLoc.x*scaleFactor);
	lua_pushnumber(ls, aLoc.y*scaleFactor);
	luaCtx_pcall(_luaCtx, 5, 0, 0);
	
	lua_pop(_luaCtx->luaState, 3);
}

- (void)setupGL
{
	[EAGLContext setCurrentContext:_context];
	
	NSString *dynamoScriptsPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"DynamoScripts"];
	NSString *localScriptsPath  = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"Scripts"];
	luaCtx_addSearchPath(_luaCtx, [dynamoScriptsPath fileSystemRepresentation]);
	luaCtx_addSearchPath(_luaCtx, [localScriptsPath fileSystemRepresentation]);
	dynamo_assert(luaCtx_executeFile(_luaCtx, [_bootScriptPath fileSystemRepresentation]), "Lua error");
}

- (void)tearDownGL
{
	[EAGLContext setCurrentContext:_context];
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
	lua_getglobal(_luaCtx->luaState, "dynamo");
	lua_getfield(_luaCtx->luaState, -1, "cycle");
	luaCtx_pcall(_luaCtx, 0, 0, 0);
	lua_pop(_luaCtx->luaState, 1);
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
