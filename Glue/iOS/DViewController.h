// Dynamo view controller

// Sets up an GLKView and initializes dynamo using the supplied boot script
#import <GLKit/GLKit.h>

extern const NSString *kDynamoMessageNotification;

@interface DViewController : GLKViewController {
    EAGLContext *_context;
    NSMutableArray *_activeTouches;
    NSString *_bootScriptPath;
}
@property(readonly) NSString *bootScriptPath;

- (id)initWithBootScriptPath:(NSString *)aPath;
@end
