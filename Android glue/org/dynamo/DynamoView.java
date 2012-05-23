package org.dynamo;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.util.AttributeSet;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLES20;
import android.util.Log;
import android.view.MotionEvent;
import org.keplerproject.luajava.LuaState;
import org.keplerproject.luajava.LuaStateFactory;

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
		public LuaState luaState;
		public String bootScriptPath;
		
		@Override
		public void onDrawFrame(GL10 unused)
		{
			this.luaState.getGlobal("dynamo");
			this.luaState.getField(-1, "cycle");
			int status = this.luaState.pcall(0, 0, 0);
			if(status != 0)
				Log.e("Dynamo", this.luaState.toString(-1));

			this.luaState.pop(1);
		}

		@Override
		public void onSurfaceChanged(GL10 unused, int width, int height)
		{
			GLES20.glViewport(0, 0, width, height);
		}

		private void _addLuaSearchPath(String searchPath)
		{
			this.luaState.getGlobal("package");
			this.luaState.getField(-1, "path");
			this.luaState.pushString(";"+searchPath+"/?.lua;"+searchPath+"/?/init.lua");
			this.luaState.concat(2);
			this.luaState.setField(-2, "path");
			this.luaState.pop(1);
		}
		
		@Override
		public void onSurfaceCreated(GL10 unused, EGLConfig config)
		{
			GLES20.glEnable(GLES20.GL_BLEND);
			GLES20.glBlendFunc(GLES20.GL_SRC_ALPHA, GLES20.GL_ONE_MINUS_SRC_ALPHA);

			// We need to initialize Dynamo after the GL surface is initialized, since it calls GL functions during initialization.
			if(this.luaState == null) {
				this.luaState=LuaStateFactory.newLuaState();
				this.luaState.openLibs();

				// Add the resource directory to the search path
				_addLuaSearchPath(ResourceManager.resourceDirPath() + "/DynamoScripts");

				if(this.luaState.LdoFile(bootScriptPath) != 0)
					Log.e("Dynamo", "Error initializing using "+bootScriptPath+": "+this.luaState.toString(-1));
			}
		}

		private void _postTouchEvent(int finger, boolean isDown, float x, float y)
		{
			this.luaState.getGlobal("dynamo");
			this.luaState.getField(-1, "input");
			this.luaState.getField(-1, "manager");
			this.luaState.getField(-1, "postTouchEvent");
			this.luaState.pushValue(-2);
			this.luaState.pushNumber(finger); // Finger
			this.luaState.pushBoolean(isDown); // Down/Not down
			this.luaState.pushNumber(x);
			this.luaState.pushNumber(y);
			if(this.luaState.pcall(5, 0, 0) != 0)
				Log.e("Dynamo", "Error posting touch event: "+this.luaState.toString(-1));
			this.luaState.pop(3);
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
