--extract-time-series.sql

-- extract time series, TEMPLATE/inspiration: vitals-first-day.sql

-- info: to choose only 1 vital: comment out both the case statement (l.14) of the other variables and the corresponding itemids below (l.71-127)


DROP MATERIALIZED VIEW IF EXISTS case_vitals CASCADE;
create materialized view case_vitals as
SELECT  pvt.icustay_id, pvt.subject_id -- removed , pvt.hadm_id,
,	pvt.chart_time 
, case 
	when pvt.chart_time < pvt.sepsis_onset then 0 
	when pvt.chart_time between pvt.sepsis_onset and (pvt.sepsis_onset+interval '5' hour ) then 1
	else null end as sepsis_target


-- Easier names

, case when VitalID = 2 then valuenum else null end as SysBP
, case when VitalID = 5 then valuenum else null end as RespRate
, case when VitalID = 1 then valuenum else null end as HeartRate

--, min(case when VitalID = 1 then valuenum else null end) as HeartRate_Min
--, max(case when VitalID = 1 then valuenum else null end) as HeartRate_Max
--, avg(case when VitalID = 1 then valuenum else null end) as HeartRate_Mean
--, min(case when VitalID = 2 then valuenum else null end) as SysBP_Min
--, max(case when VitalID = 2 then valuenum else null end) as SysBP_Max
--, avg(case when VitalID = 2 then valuenum else null end) as SysBP_Mean
--, min(case when VitalID = 3 then valuenum else null end) as DiasBP_Min
--, max(case when VitalID = 3 then valuenum else null end) as DiasBP_Max
--, avg(case when VitalID = 3 then valuenum else null end) as DiasBP_Mean
--, min(case when VitalID = 4 then valuenum else null end) as MeanBP_Min
--, max(case when VitalID = 4 then valuenum else null end) as MeanBP_Max
--, avg(case when VitalID = 4 then valuenum else null end) as MeanBP_Mean
--, min(case when VitalID = 5 then valuenum else null end) as RespRate_Min
--, max(case when VitalID = 5 then valuenum else null end) as RespRate_Max
--, avg(case when VitalID = 5 then valuenum else null end) as RespRate_Mean
--, min(case when VitalID = 6 then valuenum else null end) as TempC_Min
--, max(case when VitalID = 6 then valuenum else null end) as TempC_Max
--, avg(case when VitalID = 6 then valuenum else null end) as TempC_Mean
--, min(case when VitalID = 7 then valuenum else null end) as SpO2_Min
--, max(case when VitalID = 7 then valuenum else null end) as SpO2_Max
--, avg(case when VitalID = 7 then valuenum else null end) as SpO2_Mean
--, min(case when VitalID = 8 then valuenum else null end) as Glucose_Min
--, max(case when VitalID = 8 then valuenum else null end) as Glucose_Max
--, avg(case when VitalID = 8 then valuenum else null end) as Glucose_Mean

FROM  (
  select fd.icustay_id, ie.subject_id -- removed: ie.subject_id, ie.hadm_id, 
  , case
    when itemid in (220045) and valuenum > 0 and valuenum < 300 then 1 -- HeartRate
    when itemid in (220179,220050) and valuenum > 0 and valuenum < 400 then 2 -- SysBP
    when itemid in (8368,8440,8441,8555,220180,220051) and valuenum > 0 and valuenum < 300 then 3 -- DiasBP
    when itemid in (456,52,6702,443,220052,220181,225312) and valuenum > 0 and valuenum < 300 then 4 -- MeanBP
    when itemid in (220210,224690) and valuenum > 0 and valuenum < 70 then 5 -- RespRate
    when itemid in (223761,678) and valuenum > 70 and valuenum < 120  then 6 -- TempF, converted to degC in valuenum call
    when itemid in (223762,676) and valuenum > 10 and valuenum < 50  then 6 -- TempC
    when itemid in (646,220277) and valuenum > 0 and valuenum <= 100 then 7 -- SpO2
    when itemid in (807,811,1529,3745,3744,225664,220621,226537) and valuenum > 0 then 8 -- Glucose

    else null end as VitalID
      -- convert F to C
  , case when itemid in (223761,678) then (valuenum-32)/1.8 else valuenum end as valuenum
  , ce.charttime as chart_time
  , si.suspected_infection_time as sepsis_onset

  from final_data fd -- was icustays ie (changed it below as well)
  left join icustays ie
  	on fd.icustay_id = ie.icustay_id
  left join suspinfect_poe si
  	on fd.icustay_id = si.icustay_id -- removed: ie.subject_id = si.subject_id and ie.hadm_id = si.hadm_id and 
  left join chartevents ce
  	on fd.icustay_id = ce.icustay_id -- removed: ie.subject_id = ce.subject_id and ie.hadm_id = ce.hadm_id and 
  and ce.charttime between fd.intime and fd.outtime -- (si.si_starttime - interval '336' hour) and si.si_starttime -- (si.si_starttime + interval '5' hour) --  -- si.si_starttime -- choose chartevents between icu-entry and sepsis onset, but not more than 3 days before onset and not earlier than icu entry.
  -- and ce.charttime between (si.si_starttime - interval '72' hour) and si.si_starttime
  -- TIME RESTRICTIONS COMMENTED OUT


  -- exclude rows marked as error
 
  where fd.sepsis_case = 1 and ce.itemid in
  (
  ---- HEART RATE
  --211, --"Heart Rate"
  220045, --"Heart Rate"

  -- Systolic/diastolic

--  51, --	Arterial BP [Systolic]
--  442, --	Manual BP [Systolic]
--  455, --	NBP [Systolic]
--  6701, --	Arterial BP #2 [Systolic]
  220179, --	Non Invasive Blood Pressure systolic
  220050, --	Arterial Blood Pressure systolic

--  8368, --	Arterial BP [Diastolic]
--  8440, --	Manual BP [Diastolic]
--  8441, --	NBP [Diastolic]
--  8555, --	Arterial BP #2 [Diastolic]
--  220180, --	Non Invasive Blood Pressure diastolic
-- 220051, --	Arterial Blood Pressure diastolic


--  -- MEAN ARTERIAL PRESSURE
--  456, --"NBP Mean"
--  52, --"Arterial BP Mean"
--  6702, --	Arterial BP Mean #2
--  443, --	Manual BP Mean(calc)
--  220052, --"Arterial Blood Pressure mean"
--  220181, --"Non Invasive Blood Pressure mean"
--  225312, --"ART BP mean"

  -- RESPIRATORY RATE
--  618,--	Respiratory Rate
--  615,--	Resp Rate (Total)
  220210,--	Respiratory Rate
  224690 --, --	Respiratory Rate (Total)


  -- SPO2, peripheral
 -- 646, 220277,

  -- GLUCOSE, both lab and fingerstick
--  807,--	Fingerstick Glucose
--  811,--	Glucose (70-105)
--  1529,--	Glucose
--  3745,--	BloodGlucose
--  3744,--	Blood Glucose
--  225664,--	Glucose finger stick
--  220621,--	Glucose (serum)
--  226537,--	Glucose (whole blood)

--  -- TEMPERATURE
--  223762, -- "Temperature Celsius"
--  676,	-- "Temperature C"
--  223761, -- "Temperature Fahrenheit"
--  678 --	"Temperature F"

  )
) pvt
--group by pvt.subject_id, pvt.hadm_id, pvt.icustay_id
order by pvt.icustay_id, pvt.subject_id, pvt.chart_time; -- removed pvt.hadm_id, 




