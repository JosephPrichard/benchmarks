use std::{sync::{Condvar, Mutex}, thread::available_parallelism};

pub struct Semaphore {
    count: Mutex<u32>,
    cvar: Condvar,
}

impl Semaphore {
    pub fn with_core_count() -> Semaphore {
        let cores = available_parallelism().unwrap().get() as u32;
        Semaphore::with_count(cores)
    }
    
    pub fn with_count(count: u32) -> Semaphore {
        Semaphore{
            count: Mutex::new(count),
            cvar: Condvar::new()
        }
    }

    pub fn down_one(&self) {
        let mut count = self.count.lock().unwrap();
        while *count == 0 {
            count = self.cvar.wait(count).unwrap();
        }
        *count -= 1;
    }
    
    pub fn up_one(&self) {
        *self.count.lock().unwrap() += 1;
        self.cvar.notify_one();
    }
    
}
