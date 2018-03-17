--case-control.sql

-- apply exclusion criteria and select time series:

-- final_data: view that shows cohort with sepsis cases and controls according to SEPSIS-3 definition
-- returns the subselection of metavision-only logging, age>15.


DROP MATERIALIZED VIEW IF EXISTS final_data CASCADE; 

CREATE MATERIALIZED VIEW final_data AS   
with final_table as
(
	select sfc.icustay_id
	, sfc.SOFA_at_SI
	, sfc.SOFA_bl
	, sfc.SOFA_delta

--	, s3c.excluded
	, s3c.intime
	, s3c.outtime
	, s3c.age
    , s3c.gender
    , s3c.ethnicity
    , s3c.dbsource
    , s3c.exclusion_secondarystay
    , s3c.first_service
    , s3c.suspected_of_infection_poe
    , s3c.suspected_infection_time_poe
    , s3c.suspected_infection_time_poe_days
    , s3c.inclusion_suspicion_after_4_hours
    -- not listed atm: specimen, positive BK, antibiotic time
    , adm.HAS_CHARTEVENTS_DATA

	, case when

         s3c.age <= 14 -- CHANGED FROM ORIGINAL! <=16
         or adm.HAS_CHARTEVENTS_DATA = 0
         or s3c.intime is null
         or s3c.outtime is null
         or s3c.dbsource != 'metavision'
        -- or  s3c.suspected_of_infection_poe = 0
            then 1
        else 0 end as excluded



    FROM sepsis3_cohort s3c  
	left join sofachange sfc 
		on s3c.icustay_id = sfc.icustay_id
	left join admissions adm
    on s3c.hadm_id = adm.hadm_id

	--FROM sofachange sfc 
	--full join sepsis3_cohort s3c 
	--	on sfc.icustay_id = s3c.icustay_id
)
-- SELECT non-excluded (where exlcuded = 0)
select *
	, case when 	
		ft.suspected_of_infection_poe = 1 -- sepsis-3 criteria
			and ft.sofa_delta >= 2  -- sepsis-3 criteria 
			and ft.inclusion_suspicion_after_4_hours = 1 -- time cut-off: onset only after 4hours of icu stay
			and ft.suspected_infection_time_poe < ft.outtime -- time cut-off: onset before leaving icu
			 
			then 1
		else 0 end as sepsis_case

---- try old control definition!
----	, case when 
----		(
----		ft.suspected_of_infection_poe = 0 -- no susp of inf!
----		and ft.sofa_delta >= 2  -- sepsis-3 criteria 
----		and (ft.suspected_infection_time_poe between (ft.intime-interval '336' hour ) and ft.outtime )
----		) -- time cut-off: no onset between 1week before icu and icu-outtime!
----			then 1
----		else 0 end as sepsis_control


	, case when not 
		(
		ft.suspected_of_infection_poe = 1 -- sepsis-3 criteria
		and ft.sofa_delta >= 2  -- sepsis-3 criteria 
		and (ft.suspected_infection_time_poe between (ft.intime-interval '336' hour ) and ft.outtime )
		) -- time cut-off: no onset between 1week before icu and icu-outtime!
			then 1
		else 0 end as sepsis_control

-- setting control as susp of inf = 0 leaves NO CONTROL patient!

from final_table ft
	where ft.excluded = 0 and ft.icustay_id is not null;
--	order by ft.icustay_id;




--NEXT: EXTRACT TIMESERIES
--	inner join chartevents ce
--  on ft.icustay_id = ce.icustay_id
--  where ft.exlcuded = 0

--where si.suspected_infection_time is not null
--order by ft.icustay_id;


