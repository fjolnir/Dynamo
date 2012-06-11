package org.dynamo;

import org.dynamo.jni;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.util.AttributeSet;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLES20;
import android.util.Log;
import android.view.MotionEvent;

public class DynamoView extends GLSurfaceView {
	public DynamoRenderer renderer;
	
	public DynamoView(Context context, AttributeSet attrs) {
		super(context, attrs);
		if(isInEditMode())
			return;
		this.setEGLContextClientVersion(2);
		this.renderer = new DynamoRenderer();
		
		this.setRenderer(this.renderer);
	}

	public boolean onTouchEvent(final MotionEvent event) {
		final int pointerCount = event.getPointerCount();
		final int[] ids = new int[pointerCount];
		final float[] xs = new float[pointerCount];
		final float[] ys = new float[pointerCount];
		
		float height = this.getHeight();
		
		for(int i = 0; i < pointerCount; i++) {
			ids[i] = event.getPointerId(i);
			xs[i]  = event.getX(i);
			ys[i]  = height - event.getY(i);
		}

		switch (event.getAction() & MotionEvent.ACTION_MASK) {
		case MotionEvent.ACTION_DOWN: { // First touch
			final int id = event.getPointerId(0);
			final float x = xs[0];
			final float y = ys[0];
			queueEvent(new Runnable() { @Override public void run() { renderer.handleActionDown(id, x, y); } });
			break; }
		case MotionEvent.ACTION_POINTER_DOWN: { // Additional touch
			final int idx = event.getAction() >> MotionEvent.ACTION_POINTER_ID_SHIFT;
			final int id = event.getPointerId(idx);
			final float x = event.getX(idx);
			final float y = height - event.getY(idx);
			queueEvent(new Runnable() { public void run() { renderer.handleActionDown(id, x, y); } });
			break; }
		case MotionEvent.ACTION_MOVE: // Touch(es) moved
			queueEvent(new Runnable() { public void run() { renderer.handleActionMove(ids, xs, ys); } });
			break;
		 case MotionEvent.ACTION_POINTER_UP: { // One (out of many) touches ended
			 final int idx = event.getAction() >> MotionEvent.ACTION_POINTER_ID_SHIFT;
			 final int id = event.getPointerId(idx);
			 final float x = event.getX(idx);
			 final float y = height - event.getY(idx);
			 queueEvent(new Runnable() { public void run() { renderer.handleActionUp(id, x, y); } });
			 break; }
		 case MotionEvent.ACTION_UP: {  // Last touch ended
			 final int id = event.getPointerId(0);
			 final float x = xs[0];
			 final float y = ys[0];
			 queueEvent(new Runnable() { public void run() { renderer.handleActionUp(id, x, y); } });
			 break; }
		 case MotionEvent.ACTION_CANCEL: // Action cancelled, same as an up event, but no action should be taken
			 queueEvent(new Runnable() { public void run() { renderer.handleActionCancel(ids, xs, ys); } });
			 break;
		}

		return true;
	}

	public static class DynamoRenderer implements GLSurfaceView.Renderer
	{

		public interface MessageObserver
		{
			public void onDynamoMessage(String key, Object value);
		}
		public String bootScriptPath;
		public Runnable messageHandler;
		public MessageObserver msgObserver;
		
		@Override
		public void onDrawFrame(GL10 unused)
		{
			LuaContext_t ctx = jni.getGlobalLuaContext();
			
			jni.luaCtx_getglobal(ctx, "dynamo");
			jni.luaCtx_getfield(ctx, -1, "cycle");
			jni.luaCtx_pcall(ctx, 0, 1, 0);
			// Check if there are any messages
			if(msgObserver != null && jni.luaCtx_istable(ctx, -1) == 1) {
				jni.luaCtx_pushnil(ctx); 
				while(jni.luaCtx_next(ctx, -2) != 0) {
					// Key is at -2, value at -1
					String key = jni.luaCtx_tostring(ctx, -2);
					if(jni.luaCtx_isboolean(ctx, -1) == 1)
						msgObserver.onDynamoMessage(key, jni.luaCtx_toboolean(ctx, -1) == 1 ? true : false);
					else if(jni.luaCtx_isnumber(ctx, -1) == 1)
						msgObserver.onDynamoMessage(key, jni.luaCtx_tonumber(ctx, -1));
					else if(jni.luaCtx_isstring(ctx, -1) == 1)
						msgObserver.onDynamoMessage(key, jni.luaCtx_tostring(ctx, -1));
					else
						Log.e("Dynamo", "Unhandled message type for key "+key);
					jni.luaCtx_pop(ctx, 1); // Pop the value
				}
				jni.luaCtx_pop(ctx, 1); // Pop the key
			}
			jni.luaCtx_pop(ctx, 1);
		}

		@Override
		public void onSurfaceChanged(GL10 unused, int width, int height)
		{
			GLES20.glViewport(0, 0, width, height);
		}

		@Override
		public void onSurfaceCreated(GL10 unused, EGLConfig config)
		{
			GLES20.glEnable(GLES20.GL_BLEND);
			GLES20.glBlendFunc(GLES20.GL_SRC_ALPHA, GLES20.GL_ONE_MINUS_SRC_ALPHA);

			// We need to initialize Dynamo after the GL surface is initialized, since it calls GL functions during initialization.
			jni.luaCtx_init();
			jni.luaCtx_addSearchPath(jni.getGlobalLuaContext(), ResourceManager.resourceDirPath() + "/DynamoScripts");
			jni.luaCtx_executeFile(jni.getGlobalLuaContext(), bootScriptPath);
		}

		private void _postTouchEvent(int finger, boolean isDown, float x, float y)
		{
			jni.luaCtx_getglobal(jni.getGlobalLuaContext(), "dynamo");
 			jni.luaCtx_getfield(jni.getGlobalLuaContext(), -1, "input");
			jni.luaCtx_getfield(jni.getGlobalLuaContext(), -1, "manager");
			jni.luaCtx_getfield(jni.getGlobalLuaContext(), -1, "postTouchEvent");
			jni.luaCtx_pushvalue(jni.getGlobalLuaContext(), -2);
			jni.luaCtx_pushnumber(jni.getGlobalLuaContext(), finger);
			jni.luaCtx_pushboolean(jni.getGlobalLuaContext(), isDown ? 1 : 0);
			jni.luaCtx_pushnumber(jni.getGlobalLuaContext(), x);
			jni.luaCtx_pushnumber(jni.getGlobalLuaContext(), y);
			jni.luaCtx_pcall(jni.getGlobalLuaContext(), 5, 0, 0);
			jni.luaCtx_pop(jni.getGlobalLuaContext(), 3);
		}
		public void handleActionDown(int id, float x, float y)
		{
			_postTouchEvent(id, true, x, y);
		}

		public void handleActionUp(int id, float x, float y)
		{
			_postTouchEvent(id, false, x, y);
		}

		public void handleActionCancel(int[] id, float[] x, float[] y)
		{
			for(int i = 0; i < id.length; ++i) {
				_postTouchEvent(id[i], false, x[i], y[i]);
			}
		}

		public void handleActionMove(int[] id, float[] x, float[] y)
		{
			for(int i = 0; i < id.length; ++i) {
				_postTouchEvent(id[i], true, x[i], y[i]);
			}
		}
	}
}
