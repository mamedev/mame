package org.mamedev.mame;

import java.io.*;
import android.app.*;
import android.os.*;
import android.content.res.AssetManager;
import android.util.Log;
import org.libsdl.app.SDLActivity;
import android.view.*;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
/**
    SDL Activity
*/
public class MAME extends SDLActivity {
    private static final String TAG = "MAME";
    // Setup
    @Override
    protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		copyAssetAll("mame.ini");
		copyAssetAll("ui.ini");
		copyAssetAll("roms");
    }
	
	public void copyAssetAll(String srcPath) {
		AssetManager assetMgr = this.getAssets();
		String assets[] = null;
		try {
			String destPath = getExternalFilesDir(null) + File.separator + srcPath;
			assets = assetMgr.list(srcPath);
			if (assets.length == 0) {
				copyFile(srcPath, destPath);
			} else {
				File dir = new File(destPath);
				if (!dir.exists())
					dir.mkdir();
				for (String element : assets) {
					copyAssetAll(srcPath + File.separator + element);
				}
			}
		} 
		catch (IOException e) {
		   e.printStackTrace();
		}
	}
	public void copyFile(String srcFile, String destFile) {
		AssetManager assetMgr = this.getAssets();
	  
		InputStream is = null;
		OutputStream os = null;
		try {
			is = assetMgr.open(srcFile);
			if (new File(destFile).exists() == false)
			{
				os = new FileOutputStream(destFile);
		  
				byte[] buffer = new byte[1024];
				int read;
				while ((read = is.read(buffer)) != -1) {
					os.write(buffer, 0, read);
				}
				is.close();
				os.flush();
				os.close();
				Log.v(TAG, "copy from Asset:" + destFile);
			}
		} 
		catch (IOException e) {
			e.printStackTrace();
		}
	}
	
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {

        int keyCode = event.getKeyCode();
        // Ignore certain special keys so they're handled by Android
        if (((event.getSource() & InputDevice.SOURCE_CLASS_BUTTON) != 0) && (keyCode == KeyEvent.KEYCODE_BACK)) {
			android.os.Process.killProcess(android.os.Process.myPid());
        }
        return super.dispatchKeyEvent(event);
    }
	}
