#import <AppKit/NSWindow.h>
#import <AppKit/NSTextField.h>
#import <AppKit/NSImageView.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSFont.h>
#import <stdio.h>
#import <math.h>
#import <assert.h>

#include "cocoa_guard.h"

static BOOL about_window_exists = NO;

union RGBA {uint32_t i;unsigned char c[4];};

struct colorSpec_t {union RGBA color[2]; NSSize size;};

@interface Kashyyyk_About : NSWindow

- (NSUInteger)getWidth;
- (NSUInteger)getHeight;

- (void)close;
- (void)initData;



@property(assign) NSTextField **labels;
@property(assign) NSImage *backgroundImage;
@property(assign) NSImageView *aboutIcon;

@property(nonatomic) struct colorSpec_t colorSpec;
@property(nonatomic) CGImageRef imageRef;
@property(nonatomic) CGDataProviderRef providerRef;
@property(nonatomic) CGColorSpaceRef colorSpaceRef;
@property(nonatomic) struct CGDataProviderDirectCallbacks callbacks;

@end

static void Nothing(){}

union RGBA BlendColors(union RGBA color[2], float fraction){

    union RGBA ret;

    assert(fraction>=0.0f);
    assert(fraction<=1.0f);
    assert(color);

    for(int i = 0;i<4; i++)
        ret.c[i] = ((((float)color[0].c[i])*fraction)+ (((float)color[1].c[i])*(1.0f-fraction)));

    return ret;

}

size_t BackgroundDataCopy(void *info, void *buffer, off_t pos, size_t count){


    /* All local variables are in bytes. */
    struct colorSpec_t *color = info;
    unsigned at = 0;
    unsigned width = color->size.width*4;


    /*Debug */
    printf("%s info %p buffer %p pos %llu count %lu\n", __func__, info, buffer,
           pos, count);

    while(at<count){

        float row = ((float)(at+pos))/(width);
        union RGBA blended_color =
          BlendColors(color->color, row/color->size.height);
        /* remaining in this line. */
        unsigned remaining = width - ((pos+at)%width);
        if(remaining>count-at)
          remaining = count-at;

        memset_pattern4((char *)buffer+at, blended_color.c, remaining);

        /* Can and will shoot us past the end of the last fill position if it
        does not end on a line end.
        */
        at+=remaining;

    }

    return at;
}

void ProviderFreeWrapper(void *i, const void *data, size_t size){
    free((void *)data);
}


@implementation Kashyyyk_About : NSWindow

- (void)initData{
    _colorSpec.color[0].i = 0x804010FF;
    _colorSpec.color[1].i = 0x100080FF;
    _colorSpec.size = [self frame].size;

    {
        unsigned w = [self getWidth], h = [self getHeight];
        unsigned buffer_len = w*h*4;
        void *buffer = malloc(buffer_len);
        const unsigned line_len = [self getWidth]*4;

        size_t s = BackgroundDataCopy(&_colorSpec, buffer, 0, buffer_len);

        assert(s==buffer_len);

        _colorSpaceRef = CGColorSpaceCreateDeviceRGB();
        _callbacks.version = 0;
        _callbacks.getBytePointer =  NULL;
        _callbacks.releaseBytePointer = Nothing;
        _callbacks.getBytesAtPosition = BackgroundDataCopy;
        _callbacks.releaseInfo = Nothing;

        _providerRef = CGDataProviderCreateWithData(&_colorSpec, buffer,
                                            buffer_len, ProviderFreeWrapper);

        _imageRef = CGImageCreate(w, h, 8, 32, line_len, _colorSpaceRef,
           kCGImageAlphaLast|kCGBitmapByteOrder32Little, _providerRef, NULL, NO,
           kCGRenderingIntentDefault);
        _labels = calloc(sizeof(NSTextField *), 4);
        _labels[0] = [[NSTextField alloc] initWithFrame:
          NSMakeRect((w/4)+32, 2*h/3, (3*w/4)-16, 32)];

        _backgroundImage = [NSImage alloc];
        [_backgroundImage initWithCGImage:_imageRef size:_colorSpec.size];

        [_labels[0] setTextColor:[NSColor whiteColor]];
        [_labels[0] setEditable:NO];
        [_labels[0] setBezeled:NO];
        [_labels[0] setDrawsBackground:NO];
        [_labels[0] setStringValue: @"Kashyyyk IRC Client"];
        [_labels[0] setFont:[NSFont fontWithDescriptor: [[_labels[0] font ] fontDescriptor] size:24.0f]];
        _aboutIcon = [NSImageView alloc];
        [_aboutIcon initWithFrame:NSMakeRect(8, h-8-(w/4), w/4, w/4)];
        [_aboutIcon setImage:[NSApp applicationIconImage]];

        [[self contentView] addSubview: _aboutIcon];

        [self setBackgroundColor:[NSColor colorWithPatternImage:_backgroundImage ]];

    }
    {
        for(int i = 0;_labels[i]; i++){
            [[self contentView] addSubview: _labels[i]];
        }
    }

}

- (NSUInteger)getWidth{
    return  [[self contentView] frame].size.width;
}
- (NSUInteger)getHeight{
    return  [[self contentView] frame].size.height;
}
+ (NSUInteger)getFixedWidth{
    return  200;
}
+ (NSUInteger)getFixedHeight{
    return 400;
}

- (void)close {
    CFRelease(_providerRef);
    CFRelease(_colorSpaceRef);
    CFRelease(_imageRef);
    [super close];
    about_window_exists = NO;
}

@end

void Kashyyyk_Cocoa_AboutWindow(){


    static Kashyyyk_About *about_window = nil;

    if(!about_window_exists){

        NSUInteger style = NSTitledWindowMask|NSClosableWindowMask;

        NSRect size_rect = NSMakeRect(200, 200, 400, 200);
        NSRect win_rect = [Kashyyyk_About contentRectForFrameRect:size_rect styleMask:style];

        about_window = [Kashyyyk_About alloc];
        [about_window initWithContentRect:win_rect styleMask:style backing:NSBackingStoreBuffered defer:NO];
        [about_window initData];

        about_window.title = @"About Kashyyyk";

        about_window_exists = YES;
    }

    [about_window makeKeyAndOrderFront:NSApp];

}
