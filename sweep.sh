#!/bin/bash
# =====================================================================
#  Rock thickness sweep: for each thickness, run
#     A = rock only      ->  rockXX_only.root
#     B = rock + water   ->  rockXX_water.root
#  then compare A and B, and at the end make the summary table + plots.
#
#  Use (in the build folder):
#     bash sweep.sh
#  Run it in the background and keep it going after closing the terminal:
#     nohup bash sweep.sh > sweep.log 2>&1 &
#     tail -f sweep.log        (to watch;  Ctrl+C stops watching, not the run)
# =====================================================================

N=1000000                                   # neutrons per run (try 1000 first as a test)
THICKNESSES="0.1 0.2 0.3 0.5 0.75 1.0"      # rock thickness in metres
WATER=0.5                                   # water thickness in metres

mkdir -p logs

for T in $THICKNESSES; do
  CM=$(awk "BEGIN{printf \"%d\", $T*100+0.5}")     # 0.75 -> 75

  for W in false true; do
    if [ "$W" = "true" ]; then TAG="water"; else TAG="only"; fi
    OUT="rock${CM}_${TAG}.root"

    # write the macro for this run
    cat > sweep_tmp.mac << MAC
/vis/disable
/tracking/storeTrajectory 0
/wall/rockThickness ${T} m
/wall/waterThickness ${WATER} m
/wall/withWater ${W}
/run/initialize
/gps/particle neutron
/gps/energy 14 MeV
/gps/pos/type Point
/gps/pos/centre 0 0 -10 cm
/gps/direction 0 0 1
/run/printProgress 100000
/run/beamOn ${N}
MAC

    echo "$(date '+%H:%M:%S')  running rock ${CM} cm, water=${W}  ->  ${OUT}"
    ./sim sweep_tmp.mac > logs/rock${CM}_${TAG}.log 2>&1
    mv output0.root "${OUT}"
    grep -A4 "RESULTS" logs/rock${CM}_${TAG}.log
  done

  # compare A and B for this thickness (saves png, pdf, txt)
  root -l -b -q "compare.C(\"rock${CM}_only.root\",\"rock${CM}_water.root\",${N})" \
       > logs/compare_rock${CM}.log 2>&1
done

# one table + plots for the whole sweep
root -l -b -q "sweep_summary.C(${N})"
echo "$(date '+%H:%M:%S')  SWEEP FINISHED"
