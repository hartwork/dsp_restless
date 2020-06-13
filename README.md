# dsp_restless

## About

The _Restless Winamp Plugin_ (a.k.a. **dsp_restless**) started out in response
to a [plug-in request by users `seattlex19` and `KissFM`](http://forums.winamp.com/showthread.php?threadid=196012)
at the official Winamp community forum in 2004/2005.

Essentially the plug-in automates hitting the `Play` button
when the plug-in hasn't seen any new audio data
for a specific, customizable duration of time
so that a human doesn't have to do it themselves.

The source code of _Restless Winamp Plugin_ is released as Public Domain.


## History

### 1.3.0 (2017-07-20)

- Added: Option `Hit_Start_Only_When_Repeat_Enabled=("0"|"1")`, defaults to `"0"`;
  it was contributed by Michael Wysocki


### 1.2.0 (2007-10-30)

- Added: Delay config now possible in `<minutes>:<seconds>` as well
- Added: Pause interval (in which play is not hit)


### 1.1.2 (2007-08-08)

- Added: Delay config via `Winamp.ini`
- Added: NSIS installer


### 1.1.1 (2005-05-10)

- First version shared with the public
