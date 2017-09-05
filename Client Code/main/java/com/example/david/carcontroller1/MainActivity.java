//Client side (remote control)
//User swipes finger across screen, and the car will turn to that angle (from the vertical) and continue forward
//calculates angle (to turn) as well as speed of hand swiping across the screen, which affects the speed the car moves (speed from 1 to 10)

package com.example.david.carcontroller1;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.*;
import android.graphics.Paint;
import android.os.StrictMode;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Button;
import android.widget.Toast;

import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.io.*;


public class MainActivity extends AppCompatActivity {

    private RelativeLayout myLayout;
    private Button stop;
    TextView tmp,tmp2;

    public Socket socket,socket2;
    private static final int port = 4000;
    private static final String ip = "192.168.5.17";    //192.168.5.17
    PrintStream p,p2;

    drawLine View;
    Paint mPaint;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        myLayout = (RelativeLayout) findViewById(R.id.myLayout);
        stop = (Button) findViewById(R.id.btnStop);
        tmp = (TextView) findViewById(R.id.tv1);
        tmp2 = (TextView) findViewById(R.id.tv2);

        //to get past Android not allowing for Socket Programming in MainActivity (but our program is entirely based on this TCP IP connection)
        StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
        StrictMode.setThreadPolicy(policy);


        View = new drawLine(this);
        myLayout.addView(View);
        mPaint = new Paint();
        mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mPaint.setStyle(Paint.Style.STROKE);
        mPaint.setColor(Color.BLACK);
        mPaint.setStrokeWidth(30);

        stop.setOnTouchListener(new View.OnTouchListener(){
            @Override
            public boolean onTouch(View v, MotionEvent event) {

                try{

                    tmp.setText("clicked");
                    //Toast.makeText(getApplicationContext(),"socket", Toast.LENGTH_SHORT).show();
                    InetAddress serverAddr = InetAddress.getByName(ip);
                    socket = new Socket(serverAddr,port);
                    p = new PrintStream(socket.getOutputStream());
                    p.println("STOP");
                }
                catch(IOException e){
                    e.printStackTrace();
                }

                return true;
            }
        });
    }




    public class drawLine extends View{

        float x0,y0,x1,y1;
        float angle,d,speed;
        long t0,t1;
        int level;

        public drawLine(Context context){
                super(context);
        }

        @Override
        protected void onDraw(Canvas canvas){
            super.onDraw(canvas);
            canvas.drawLine(x0,y0,x1,y1,mPaint);
        }


        @Override
        public boolean onTouchEvent(MotionEvent event){

            if(event.getAction() == MotionEvent.ACTION_DOWN){
                x0 = event.getX();
                y0 = event.getY();
                t0 = System.currentTimeMillis();
                invalidate();

            }

            if(event.getAction() == MotionEvent.ACTION_MOVE){
                x1 = event.getX();
                y1 = event.getY();
                invalidate();
            }

            if(event.getAction() == MotionEvent.ACTION_UP){
                x1 = event.getX();
                y1 = event.getY();
                d = (float)Math.sqrt( (x1-x0)*(x1-x0) + (y1-y0)*(y1-y0) );

                if(d!=0)
                {

                    invalidate();

                    t1 = System.currentTimeMillis();
                    long t = t1-t0;     //elapsed time

                    d = (float)Math.sqrt( (x1-x0)*(x1-x0) + (y1-y0)*(y1-y0) );
                    speed = d/t;        //min  0  max 15


                    float m = Math.abs((y1-y0)/(x1-x0));

                    if(x1 >= x0 && y1 <= y0){   //q1    make negative
                        angle = -(90 - (float) Math.toDegrees(Math.atan(m)));
                    }
                    else if(x1 <= x0 && y1 <= y0){  //q2
                        angle = 360 - ((float) Math.toDegrees(Math.atan(m)) + 270);
                    }
                    else if(x1 <= x0 && y1 >= y0){  //q3
                        angle = 360 - ((90 - (float) Math.toDegrees(Math.atan(m)))+180);
                    }
                    else if(x1 >= x0 && y1 >= y0){  //q4
                        angle = -((float) Math.toDegrees(Math.atan(m)) + 90);
                    }

                    if(speed <= 1.5)
                        level = 1;  //no move
                    else if(speed >= 15)
                        level = 10; //max speed 10

                    else{
                        level = (int) (speed/15*10);
                    }

                    int Angle = (int)angle;
                    String out = "TURN ANGLE " + Angle + " SPEED " + level;

                    tmp2.setText(out);

                    //Toast.makeText(getApplicationContext(),socket.isConnected()+"", Toast.LENGTH_SHORT).show();

                    try{


                        socket = new Socket(ip,port);
                        p = new PrintStream(socket.getOutputStream());

                        p.println(out);
                        p.flush();
                    } catch(IOException e){
                        e.printStackTrace();
                    }

                }
            }
            return true;
        }
    }


}
