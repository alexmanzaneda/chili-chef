from PIL import Image
import sys, random
O=(17,8,22,255)
S_HI=(236,234,236,255); S_LT=(206,205,212,255); S_MD=(166,164,178,255); S_DK=(118,114,132,255); S_DD=(78,72,92,255)
P_BG=(46,40,58,255); P_LT=(70,64,86,255)
F0=(255,248,196,255); F1=(255,220,80,255); F2=(255,160,36,255); F3=(234,88,32,255); F4=(170,36,28,255); F5=(100,18,26,255)
WH=(253,251,247,255); RED=(150,16,20,255)
HOT=(196,92,62,255); HOT2=(150,70,60,255)
K_ON=(255,120,30,255); K_ON2=(255,210,90,255); K_OFF=(60,56,70,255)

def new(w,h): return Image.new('RGBA',(w,h),(0,0,0,0))
def px(im,x,y,c):
    if 0<=x<im.width and 0<=y<im.height: im.putpixel((x,y),c)
def rect(im,x0,y0,x1,y1,c):
    for y in range(y0,y1+1):
        for x in range(x0,x1+1): px(im,x,y,c)
def outline(im):
    w,h=im.size; src=im.copy()
    for y in range(h):
        for x in range(w):
            if src.getpixel((x,y))[3]==0:
                for dx,dy in ((1,0),(-1,0),(0,1),(0,-1)):
                    xx,yy=x+dx,y+dy
                    if 0<=xx<w and 0<=yy<h and src.getpixel((xx,yy))[3]>0 and src.getpixel((xx,yy))!=O:
                        im.putpixel((x,y),O); break

def flame_tongues(im, base_y, heights, x0, phase):
    # heights: list of column heights starting at x0
    for i,hgt in enumerate(heights):
        x=x0+i
        for k in range(hgt):
            y=base_y-k
            t=k/max(hgt,1)
            # colour by distance from tip and from column center
            c = F4 if t<0.25 else F3 if t<0.5 else F2 if t<0.75 else F1
            px(im,x,y,c)
    # inner core
    for i,hgt in enumerate(heights):
        x=x0+i
        core=max(0,hgt-6)
        for k in range(core):
            y=base_y-k
            if 0<i<len(heights)-1 and heights[i-1]>4 and heights[i+1]>4:
                t=k/max(core,1)
                px(im,x,y,F1 if t<0.6 else F0)

def boss(lives=5, frame=0, hurt=False):
    im=new(64,64)
    # ---- flames on top (behind body) ----
    sets=[
      [3,5,8,11,9,6,8,12,14,12,9,7,9,12,10,8,10,13,11,8,6,9,12,10,7,5,8,11,13,11,8,6,8,10,7,5,3,4],
      [2,4,7,10,12,9,7,10,13,11,8,6,8,11,13,10,9,12,14,10,7,8,11,9,6,7,10,12,11,9,7,7,9,8,6,4,2,3],
      [3,6,9,10,8,7,9,13,12,10,8,8,11,13,11,8,9,11,12,9,7,10,13,11,8,6,9,10,12,10,8,5,7,9,8,6,4,2],
      [2,5,8,12,11,8,6,9,12,13,11,7,8,10,12,11,10,13,12,10,8,9,10,12,9,7,8,12,13,10,7,6,9,11,8,5,3,3],
    ]
    flame_tongues(im, 15, sets[frame%4], 13, frame)
    # ---- body ----
    bx0,by0,bx1,by1=7,14,56,55
    rect(im,bx0,by0,bx1,by1,S_LT)
    # rounded corners
    for (x,y) in [(bx0,by0),(bx1,by0),(bx0,by1),(bx1,by1)]: px(im,x,y,(0,0,0,0))
    # shading: top highlight, right side dark, bottom dark
    rect(im,bx0+1,by0,bx1-1,by0,S_HI)
    rect(im,bx0,by0+1,bx0,by1-1,S_HI)
    rect(im,bx1-3,by0+1,bx1,by1-1,S_MD)
    rect(im,bx1,by0+1,bx1,by1-1,S_DK)
    rect(im,bx0+1,by1-2,bx1-1,by1,S_MD)
    rect(im,bx0+1,by1,bx1-1,by1,S_DK)
    # heat tint at the bottom (it's hot!)
    for x in range(bx0+1,bx1):
        if (x*7+frame)%5<3: px(im,x,by1-1,HOT)
        px(im,x,by1,HOT2 if x%3 else S_DK)
    # ---- control panel with 5 knobs (lives) ----
    rect(im,bx0+2,16,bx1-2,23,O)
    rect(im,bx0+3,17,bx1-3,22,P_BG)
    rect(im,bx0+3,17,bx1-3,17,P_LT)
    knob_x=[14,22,30,38,46]
    for i,kx in enumerate(knob_x):
        on = i<lives
        # 4x4 round knob with outline
        for (dx,dy) in [(1,0),(2,0),(0,1),(3,1),(0,2),(3,2),(1,3),(2,3)]: px(im,kx+dx,18+dy,O)
        fill = K_ON if on else K_OFF
        for (dx,dy) in [(1,1),(2,1),(1,2),(2,2)]: px(im,kx+dx,18+dy,fill)
        if on: px(im,kx+1,19,K_ON2)
        else: px(im,kx+1,19,P_LT)
    # little temperature display on right side of panel
    rect(im,52,18,53,21,(255,60,40,255) if frame%2==0 else (200,30,30,255))
    # ---- angry eyes ----
    EYE=["###.......",
         ".####.....",
         "#WW####...",
         "#WWWWW####",
         "#WWWRRWWW#",
         "#WWWRRWWW#",
         ".########."]
    def eye(x0, flip):
        for r,row in enumerate(EYE):
            for c,ch in enumerate(row):
                cc = (9-c) if flip else c
                y=25+r; x=x0+cc
                if ch=='#': px(im,x,y,O)
                elif ch=='W': px(im,x,y,WH)
                elif ch=='R': px(im,x,y,O if hurt else RED)
        if not hurt:
            px(im,x0+(5 if flip else 4),29,(225,50,45,255))
        else:
            for y in (29,30):
                px(im,x0+(4 if flip else 5),y,O); px(im,x0+(5 if flip else 4),y,O)
    eye(14,False); eye(40,True)
    # ---- door with glowing window = mouth ----
    dx0,dy0,dx1,dy1=11,36,52,53
    rect(im,dx0,dy0,dx1,dy1,O)
    rect(im,dx0+1,dy0+1,dx1-1,dy1-1,S_MD)
    rect(im,dx0+1,dy0+1,dx1-1,dy0+1,S_HI)
    rect(im,dx0+1,dy1-1,dx1-1,dy1-1,S_DK)
    # handle bar
    rect(im,18,33,45,33,O); rect(im,17,34,17,35,O); rect(im,46,34,46,35,O)
    rect(im,18,35,45,35,O); rect(im,18,34,45,34,S_HI)
    # window
    wx0,wy0,wx1,wy1=15,39,48,50
    rect(im,wx0,wy0,wx1,wy1,O)
    rng=random.Random(frame)
    for y in range(wy0+1,wy1):
        for x in range(wx0+1,wx1):
            # vertical fire gradient, bright at bottom center
            cx=(x-(wx0+wx1)/2)/((wx1-wx0)/2)
            t=(y-wy0)/(wy1-wy0) + (0.15 if abs(cx)<0.5 else 0) + rng.uniform(-0.12,0.12)
            c = F5 if t<0.3 else F4 if t<0.48 else F3 if t<0.66 else F2 if t<0.82 else F1
            if hurt: c = F1 if t>0.5 else F2
            px(im,x,y,c)
    # flame licks inside window
    for lx in [20,27,33,40,45]:
        h=3+((lx+frame*3)%4)
        for k in range(h): px(im,lx,wy1-1-k,F1 if k<h-1 else F2)
        if h>3: px(im,lx,wy1-1,F0)
    # teeth (top & bottom) -> mouth
    TSH=(206,200,206,255)
    for tx in range(wx0+2,wx1-4,6):
        for k,w in enumerate([5,3,1]):
            for d in range(w): px(im,tx+k+d,wy0+1+k,WH)
            px(im,tx+k+w-1,wy0+1+k,TSH)
    for tx in (21,30,39):
        for k,w in enumerate([3,1]):
            for d in range(w): px(im,tx+k+d,wy1-1-k,WH)
    # door glass glare
    px(im,wx0+2,wy0+4,F1); 
    # ---- side vent on the right ----
    for y in (37,40,43,46): rect(im,54,y,55,y,S_DD)
    # ---- legs ----
    for lx in (14,43):
        rect(im,lx,56,lx+6,58,S_DD)
        rect(im,lx-1,59,lx+7,60,S_DK)
        rect(im,lx,59,lx+3,59,S_MD)
    outline(im)
    # ---- smoke puffs ----
    puffs=[[(9,6),(10,5),(10,6),(11,6)],[(53,4),(54,4),(54,3),(55,4)],[(8,4),(9,3),(9,4)],[(54,7),(55,6),(55,7),(56,7)]]
    for (x,y) in puffs[frame%4]:
        if im.getpixel((x,y))[3]==0: px(im,x,y,(120,112,128,200))
    # embers
    for (x,y) in [(6+frame*2%5,10-frame%3),(57-frame%4,12-frame%2),(30+frame,0+frame%2)]:
        if im.getpixel((x,y))[3]==0: px(im,x,y,F2)
    return im

def minion(frame=0):
    im=new(16,16)
    shapes=[
      [0,0,0,4,7,9,12,13,11,9,12,8,5,0,0,0],
      [0,0,0,5,8,11,13,12,10,11,9,7,4,0,0,0],
      [0,0,0,4,6,10,11,13,12,10,11,9,5,0,0,0],
      [0,0,0,5,9,12,11,10,13,12,9,6,4,0,0,0]]
    hs=shapes[frame%4]
    base=14
    for x,h in enumerate(hs):
        for k in range(h):
            y=base-k; t=k/h
            c=F4 if t>0.8 else F3 if t>0.6 else F2
            px(im,x,y,c)
    # bottom round
    rect(im,4,14,11,14,F3)
    # core
    for x in range(5,11):
        for y in range(9,14): px(im,x,y,F1)
    for x in range(6,10):
        for y in range(11,14): px(im,x,y,F0)
    outline(im)
    # angry eyes
    for ex,fl in ((5,False),(9,True)):
        px(im,ex,10,WH); px(im,ex+1,10,WH)
        px(im,ex+(1 if not fl else 0),10,O)
        # brow
        if not fl: px(im,ex,9,O); px(im,ex+1,9,F1); 
        else: px(im,ex+1,9,O)
    px(im,6,9,O) ; px(im,9,9,O)
    return im

if __name__=='__main__':
    import os
    out='/home/claude/boss/out'; os.makedirs(out,exist_ok=True)
    b=boss(5,0); b.save(f'{out}/oven_boss.png')
    sheet=new(64*4,64)
    for f in range(4): sheet.paste(boss(5,f),(64*f,0))
    sheet.save(f'{out}/oven_boss_idle_sheet.png')
    lives=new(64*6,64)
    for l in range(5,-1,-1): lives.paste(boss(l,0),(64*(5-l),0))
    lives.save(f'{out}/oven_boss_lives_5_to_0.png')
    boss(4,0,hurt=True).save(f'{out}/oven_boss_hurt.png')
    m=new(64,16)
    for f in range(4): m.paste(minion(f),(16*f,0))
    m.save(f'{out}/flame_minion_sheet.png'); minion(0).save(f'{out}/flame_minion.png')
